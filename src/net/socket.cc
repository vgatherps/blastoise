#include "net/socket.hh"

namespace blastoise::net {

seastar::future<> UdpSocket::send(seastar::net::packet packet) {
  return channel.send(addr, std::move(packet));
}
} // namespace blastoise::net
