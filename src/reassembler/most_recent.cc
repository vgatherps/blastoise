
#include "reassembler/most_recent.hh"

namespace blastoise {
namespace reassembler {

ForwardResult
MostRecentReassembler::handle_reliable(Packet packet,
                                       PacketSequence last_forwarded) {
  if (packet.sequence.ahead_of(last_forwarded)) {
    return Forward{std::move(packet)};
  } else {
    return SkipPacket{};
  }
}

ForwardResult MostRecentReassembler::handle_unreliable(
    Packet packet, PacketSequence last_forwarded, PacketSequence) {
  return handle_reliable(std::move(packet), last_forwarded);
}

} // namespace reassembler
} // namespace blastoise