#include "sender/packet_broadcaster.hh"

#include <seastar/core/coroutine.hh>
#include <seastar/coroutine/all.hh>

#include <coroutine>

namespace blastoise::net {

PacketBroadcaster::PacketBroadcaster()
    : udp_forwarder(BatchedSender::make_batched_sender(
          SendingSocketManager::make_socket_manager(10),
          std::chrono::microseconds(30), 1400)),
      tcp_forwarder(BatchedSender::make_batched_sender(
          // tcp MUST be one as individual sends must be totally ordered
          SendingSocketManager::make_socket_manager(1),
          // TCP also sends much less frequently, and will send packets less
          // frequently to reduce overhead outside of the networking stack.
          //
          // TCP also has no batch size limit, to accomodate arbitrarily large
          // packets
          // There's some overflow risk from a malicious or malformed sender...
          std::chrono::microseconds(10 * 1000), -1)) {}

seastar::future<> PacketBroadcaster::broadcast_packet(seastar::net::packet p) {
  seastar::net::packet udp_p = p.share();
  auto [_udp_success, tcp_success] = co_await seastar::coroutine::all(
      [&]() { return udp_forwarder->deposit_packet(std::move(udp_p)); },
      [&]() { return tcp_forwarder->deposit_packet(std::move(p)); });

  // TODO log udp success rate in theory

  if (!tcp_success) {
    throw std::runtime_error("TCP packet deposits must always succeed");
  }

  check_for_dead_clients();
}

void PacketBroadcaster::check_for_dead_clients() {
  for (SendFailure f : udp_forwarder->get_failed_sockets()) {
    clear_client(f);
  }

  for (SendFailure f : tcp_forwarder->get_failed_sockets()) {
    clear_client(f);
  }
}

void PacketBroadcaster::clear_client(SendFailure send) {
  // log error from send failure

  udp_forwarder->remove_client(send.failed_id);
  tcp_forwarder->remove_client(send.failed_id);
}

} // namespace blastoise::net