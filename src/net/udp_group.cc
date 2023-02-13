#include "net/udp_group.hh"

namespace blastoise::net {

void UdpGroup::send_to_all(UdpPacket &packet) {
  // as far as I can tell, you *do not* need to await a seastar future
  // for it to be run to completion? Hence just schedule and ignore
  // the UDP sends since they must be in order

  // If not, need to create some sort of awaiter. for UDP happy to just
  // fire and forget for now, although would want to try and detect
  // whether we have to block (and should throttle). However
  // real limits on small lines come far before machine limits

  for (auto &[id, socket] : sockets) {
    // TODO confirm (via source or testing)
    // that a dropped future will still run to completion?
    // otherwise need to schedule some wacky sort of
    // packet-sending-awaiter
    auto fire_and_forget = socket.send(packet);
  }
}

} // namespace blastoise::net