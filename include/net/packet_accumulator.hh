#pragma once

#include "seastar/net/packet.hh"

namespace blastoise::net {
class PacketAccumulator {
  seastar::net::packet buffer;

public:
  std::size_t size() { return buffer.len(); }

  void append_packet(seastar::net::packet victim);

  seastar::net::packet release_packet();
};
} // namespace blastoise::net