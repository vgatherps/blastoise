#include "net/udp/udp_socket.hh"

namespace blastoise::net {

seastar::future<> UdpSocket::send(UdpPacket &packet) {
  return channel.send(addr, packet.create_packet());
}
} // namespace blastoise::net
