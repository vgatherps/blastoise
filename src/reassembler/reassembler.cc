#include "reassembler/reassembler.hh"
#include "reassembler/forward_all.hh"
#include "reassembler/fully_ordered.hh"
#include "reassembler/most_recent.hh"

#include <scelta.hpp>
#include <seastar/net/packet.hh>

#include <algorithm>
#include <concepts>
#include <deque>
#include <optional>

namespace blastoise {
namespace reassembler {

template <class R, class Storage>
concept CPacketReassembler = requires(R r, Packet<Storage> p,
                                      PacketSequence last_forwarded) {
  {
    r.handle_reliable(std::move(p), last_forwarded)
    } -> std::same_as<ForwardResult<Storage>>;
}
&&requires(R r, Packet<Storage> p, PacketSequence last_forwarded,
           PacketSequence last_reliable) {
  {
    r.handle_unreliable(std::move(p), last_forwarded, last_reliable)
    } -> std::same_as<ForwardResult<Storage>>;
}
&&std::is_default_constructible_v<R>
    &&std::same_as<const bool, decltype(R::HasOrderedPackets)>;

template <class Storage, CPacketReassembler<Storage> R>
struct ReassemblyBuffer {
  R reassembler;
  PacketSequence last_forwarded, last_reliable;
  std::deque<PacketSequence> forwarded_unreliable;

  ReassemblyBuffer(PacketSequence first_reliable)
      : last_forwarded(first_reliable), last_reliable(first_reliable) {}

  ForwardResult<Storage> handle_unreliable(Packet<Storage> p) {
    ForwardResult<Storage> forward = reassembler.handle_unreliable(
        std::move(p), last_forwarded, last_reliable);
    scelta::match(
        [this](SkipPacket) {},

        [this](const Forward<Storage> &f) {
          if constexpr (R::HasOrderedPackets) {
            this->forwarded_unreliable.push_back(f.packet.sequence);
          } else {
            // Find the insertion position by insertion after the
            // first packet that the inbound one is ahead of
            auto it = std::find_if(forwarded_unreliable.rbegin(),
                                   forwarded_unreliable.rend(),
                                   [&f](const PacketSequence &s) {
                                     return f.packet.sequence.ahead_of(s);
                                   });
            this->forwarded_unreliable.insert(it.base(), f.packet.sequence);
          }
        },

        // assumption here - we know that these are only created
        // by fully ordered streams, so we can avoid any sort of
        // ordering overhead
        [this](const BulkForward<Storage> &b) {
          if constexpr (!R::HasOrderedPackets) {
            throw std::runtime_error(
                "BulkForward should only be used with fully ordered "
                "reassemblers");
          }
          this->forwarded_unreliable.push_back(b.head.sequence);
          for (const auto &packet : b.buffered) {
            this->forwarded_unreliable.push_back(packet.sequence);
          }
        })(forward);
    update_last_forwarded(forward);
    return forward;
  }

  ForwardResult<Storage> handle_reliable(Packet<Storage> p) {
    PacketSequence reliable_sequence = p.sequence;
    if (!reliable_sequence.succeeds(last_reliable)) [[unlikely]] {
      throw std::runtime_error("Reliable packets delivered out of order");
    }
    ForwardResult<Storage> forward =
        reassembler.handle_reliable(std::move(p), last_forwarded);
    while (!forwarded_unreliable.empty()) {
      const auto &front = forwarded_unreliable.front();

      if (front.sequence > reliable_sequence.sequence) {
        break;
      } else if (front.sequence <= reliable_sequence.sequence) {
        if (front.sequence == reliable_sequence.sequence &&
            front.hash != reliable_sequence.hash) [[unlikely]] {
          throw std::runtime_error(
              "Found hash mismatch between reliable and unrealiable packets");
        }
        forwarded_unreliable.pop_front();
      }
    }
    update_last_forwarded(forward);
    last_reliable = reliable_sequence;
    return forward;
  }

  void update_last_forwarded(const ForwardResult<Storage> &forward) {

    scelta::match([this](SkipPacket) {},
                  [this](const Forward<Storage> &f) {
                    this->last_forwarded = f.packet.sequence;
                  },
                  [this](const BulkForward<Storage> &b) {
                    this->last_forwarded = b.buffered.back().sequence;
                  });
  }
};

template <class Storage, CPacketReassembler<Storage> R>
class PacketReassembler : public Reassembler<Storage> {
  std::optional<ReassemblyBuffer<Storage, R>> reassembler;

public:
  ForwardResult<Storage> handle_unreliable(Packet<Storage> p) override {
    if (!reassembler.has_value()) [[unlikely]] {
      return SkipPacket{};
    }

    return reassembler->handle_unreliable(std::move(p));
  }
  ForwardResult<Storage> handle_reliable(Packet<Storage> p) override {
    if (!reassembler.has_value()) [[unlikely]] {
      reassembler = ReassemblyBuffer<Storage, R>(p.sequence);
      return Forward<Storage>{std::move(p)};
    }

    return reassembler->handle_reliable(std::move(p));
  }
};

template <class Storage>
std::unique_ptr<Reassembler<Storage>>
do_create_reassembler(protocol::StreamType type) {
  switch (type) {
  case protocol::StreamType::Ordered:
  case protocol::StreamType::Depth:
    return std::make_unique<
        PacketReassembler<Storage, MostRecentReassembler<Storage>>>();
  case protocol::StreamType::ReceiveAll:
    return std::make_unique<
        PacketReassembler<Storage, FullyOrderedReassembler<Storage>>>();
  case protocol::StreamType::BBO:
    return std::make_unique<
        PacketReassembler<Storage, FullyOrderedReassembler<Storage>>>();
  }
  // Unreachable
  throw std::runtime_error(
      "Impossible enum value reached in creation function");
}

template <>
std::unique_ptr<Reassembler<std::vector<std::uint8_t>>>
create_reassembler(protocol::StreamType t) {
  return do_create_reassembler<std::vector<std::uint8_t>>(t);
}

template <>
std::unique_ptr<Reassembler<seastar::net::packet>>
create_reassembler(protocol::StreamType t) {
  return do_create_reassembler<seastar::net::packet>(t);
}

} // namespace reassembler
} // namespace blastoise