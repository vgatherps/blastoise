#pragma once

#include "seastar/net/packet.hh"

namespace blastoise::net {
class TcpPacketAccumulator {
  seastar::net::packet buffer;

public:
  void append_packet(seastar::net::packet victim);

  seastar::net::packet release_packet();
};
} // namespace blastoise::net