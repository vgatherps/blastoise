#include "reassembler/reassembler.hh"
#include "reassembler/forward_all.hh"
#include "reassembler/fully_ordered.hh"
#include "reassembler/most_recent.hh"

#include <concepts>
#include <deque>
#include <optional>

#include <scelta.hpp>
#include <seastar/net/tcp.hh>

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
&&std::is_default_constructible_v<R>;

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
    scelta::match([this](SkipPacket) {},
                  [this](const Forward<Storage> &f) {
                    this->forwarded_unreliable.push_back(f.packet.sequence);
                  },
                  [this](const BulkForward<Storage> &b) {
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
do_create_reassembler(ForwarderType type) {
  switch (type) {
  case ForwarderType::MostRecent:
    return std::make_unique<
        PacketReassembler<Storage, MostRecentReassembler<Storage>>>();
  case ForwarderType::ForwardAll:
    return std::make_unique<
        PacketReassembler<Storage, FullyOrderedReassembler<Storage>>>();
  case ForwarderType::FullyOrdered:
    return std::make_unique<
        PacketReassembler<Storage, FullyOrderedReassembler<Storage>>>();
  }
  // Unreachable
  throw std::runtime_error(
      "Impossible enum value reached in creation function");
}

template <>
std::unique_ptr<Reassembler<std::vector<std::uint8_t>>>
create_reassembler(ForwarderType t) {
  return do_create_reassembler<std::vector<std::uint8_t>>(t);
}

template <>
std::unique_ptr<Reassembler<seastar::net::packet>>
create_reassembler(ForwarderType t) {
  return do_create_reassembler<seastar::net::packet>(t);
}

} // namespace reassembler
} // namespace blastoise