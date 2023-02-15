#include "net/packet_accumulator.hh"

namespace blastoise::net {
void PacketAccumulator::append_packet(seastar::net::packet p) {
  buffer.append(std::move(p));
}

seastar::net::packet PacketAccumulator::release_packet() {
  seastar::net::packet replacement(buffer.nr_frags());
  std::swap(buffer, replacement);
  return replacement;
}
} // namespace blastoise::net