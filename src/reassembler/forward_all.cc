
#include "reassembler/forward_all.hh"

namespace blastoise {
namespace reassembler {

ForwardResult
ForwardAllReassembler::handle_reliable(Packet packet,
                                       PacketSequence last_forwarded) {
  auto removed = unconfirmed_packets.erase(packet.sequence.sequence);

  if (removed) {
    return SkipPacket{};
  } else {
    return Forward{std::move(packet)};
  }
}

ForwardResult
ForwardAllReassembler::handle_unreliable(Packet packet,
                                         PacketSequence last_forwarded,
                                         PacketSequence last_reliable) {

  if (last_reliable.implies_already_seen(packet.sequence)) {
    return SkipPacket{};
  }

  auto [iter, inserted] = unconfirmed_packets.insert(packet.sequence.sequence);

  if (inserted) {
    return Forward{std::move(packet)};
  } else {
    return SkipPacket{};
  }
}

} // namespace reassembler
} // namespace blastoise