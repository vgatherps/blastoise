#pragma once

#include "net/udp_packet.hh"

#include <seastar/net/api.hh>

namespace blastoise::net {
class UdpSocket {
  seastar::net::udp_channel channel;
  seastar::socket_address addr;

public:
  seastar::future<> send(UdpPacket &packet);
};
} // namespace blastoise::net
