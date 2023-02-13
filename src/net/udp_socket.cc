#include "net/udp_group.hh"

namespace blastoise::net {

void UdpGroup::send_to_all(const UdpPacket &packet) {
  // as far as I can tell, you *do not* need to await a seastar future
  // for it to be run to completion? Hence just schedule and ignore
  // the UDP sends since they must be in order

  // If not, need to create some sort of awaiter. for UDP happy to just
  // fire and forget for now, although would want to try and detect
  // whether we have to block (and should throttle). However
  // real limits on small lines come far before machine limits

  for (auto &[id, socket] : sockets) {
    socket.send(packet);
  }
}

} // namespace blastoise::net