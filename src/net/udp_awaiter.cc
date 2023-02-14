#include "net/udp_awaiter.hh"

namespace blastoise::net {
  UdpAwaiter::UdpAwaiter(seastar::pipe_reader<PacketBatch> p) : incoming_packet_batches(std::move(p)) {}
}
