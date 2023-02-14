#pragma once

#include <seastar/net/api.hh>
#include <seastar/net/packet.hh>

namespace blastoise::net {
class Socket {
  seastar::net::udp_channel channel;
  seastar::socket_address addr;

public:
  seastar::future<> send(seastar::net::packet packet);
};
} // namespace blastoise::net
