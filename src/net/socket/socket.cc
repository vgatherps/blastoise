#include "net/socket/socket.hh"

namespace blastoise::net {

seastar::future<> Socket::send(seastar::net::packet packet) {
  return channel.send(addr, std::move(packet));
}
} // namespace blastoise::net
