#include "net/tcp/tcp_packet_accumulator.hh"

namespace blastoise::net {
void TcpPacketAccumulator::append_packet(seastar::net::packet p) {
  buffer.append(std::move(p));
}

seastar::net::packet TcpPacketAccumulator::release_packet() {
  seastar::net::packet replacement(buffer.nr_frags());
  std::swap(buffer, replacement);
  return replacement;
}
} // namespace blastoise::net