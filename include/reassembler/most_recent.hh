#pragma once

#include "reassembler.hh"

namespace blastoise {
namespace reassembler {

/// This reassembler forwards anything after the most recent sequence
template <class Storage>
class MostRecentReassembler final : public Reassembler<Storage> {
public:
  ForwardResult<Storage>
  handle_reliable(Packet<Storage> packet,
                  PacketSequence last_forwarded) override;

  ForwardResult<Storage>
  handle_unreliable(Packet<Storage> packet, PacketSequence last_forwarded,
                    PacketSequence last_reliable) override;
};

template <class Storage>
ForwardResult<Storage>
MostRecentReassembler<Storage>::handle_reliable(Packet<Storage> packet,
                                                PacketSequence last_forwarded) {
  if (packet.sequence.ahead_of(last_forwarded)) {
    return Forward<Storage>{std::move(packet)};
  } else {
    return SkipPacket{};
  }
}

template <class Storage>
ForwardResult<Storage> MostRecentReassembler<Storage>::handle_unreliable(
    Packet<Storage> packet, PacketSequence last_forwarded, PacketSequence) {
  return handle_reliable(std::move(packet), last_forwarded);
}
} // namespace reassembler
} // namespace blastoise