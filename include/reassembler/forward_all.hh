#pragma once

#include "reassembler.hh"

// stl implementation is quite slow but that's irrelevant for a hobby project
#include <unordered_set>

namespace blastoise {
namespace reassembler {

/// This reassembler forwards one copy of every received packet, in
/// arbitrary order
template <class Storage> class ForwardAllReassembler {

  std::unordered_set<PacketSeqNo> unconfirmed_packets;

public:
  constexpr static bool HasOrderedPackets = false;
  ForwardResult<Storage> handle_reliable(Packet<Storage> packet,
                                         PacketSequence last_forwarded);

  ForwardResult<Storage> handle_unreliable(Packet<Storage> packet,
                                           PacketSequence last_forwarded,
                                           PacketSequence last_reliable);
};

template <class Storage>
ForwardResult<Storage>
ForwardAllReassembler<Storage>::handle_reliable(Packet<Storage> packet,
                                                PacketSequence last_forwarded) {
  auto removed = unconfirmed_packets.erase(packet.sequence.sequence);

  if (removed) {
    return SkipPacket{};
  } else {
    return Forward<Storage>{std::move(packet)};
  }
}

template <class Storage>
ForwardResult<Storage> ForwardAllReassembler<Storage>::handle_unreliable(
    Packet<Storage> packet, PacketSequence last_forwarded,
    PacketSequence last_reliable) {

  if (last_reliable.implies_already_seen(packet.sequence)) {
    return SkipPacket{};
  }

  auto [iter, inserted] = unconfirmed_packets.insert(packet.sequence.sequence);

  if (inserted) {
    return Forward<Storage>{std::move(packet)};
  } else {
    return SkipPacket{};
  }
}
} // namespace reassembler
} // namespace blastoise