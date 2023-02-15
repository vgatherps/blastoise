#pragma once

#include <seastar/net/api.hh>
#include <seastar/net/packet.hh>

namespace blastoise::net {
class Socket {
public:
  virtual seastar::future<> send(seastar::net::packet packet) = 0;
};

class UdpSocket : public Socket {
  seastar::net::udp_channel channel;
  seastar::socket_address addr;

public:
  seastar::future<> send(seastar::net::packet packet) override;
};
} // namespace blastoise::net
