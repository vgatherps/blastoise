#include "net/batched_sender.hh"

#include <seastar/core/future.hh>

namespace blastoise::net {
BatchedSender::BatchedSender(
    seastar::lw_shared_ptr<SendingSocketManager> manager)
    : sockets(std::move(manager)) {}

void BatchedSender::deposit_packet(seastar::net::packet packet) {
  pending_packets.append_packet(std::move(packet));
}

std::optional<seastar::future<seastar::future<>>> BatchedSender::send_batch() {
  if (pending_packets.size() > 0) {
    return sockets->send_to_all(pending_packets.release_packet());
  } else {
    return {};
  }
}

} // namespace blastoise::net