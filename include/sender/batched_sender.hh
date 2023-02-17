#pragma once

#include "sender/packet_accumulator.hh"
#include "sender/sending_socket_manager.hh"

#include <seastar/core/shared_ptr.hh>

#include <chrono>

namespace blastoise::net {
class BatchedSender
    : public seastar::enable_lw_shared_from_this<BatchedSender> {
  seastar::lw_shared_ptr<SendingSocketManager> sockets;
  PacketAccumulator pending_packets;

  std::chrono::microseconds resend_time;
  std::size_t max_batch_size;
  std::uint64_t send_sequence;

  bool needs_timer_scheduled;

  friend seastar::lw_shared_ptr<BatchedSender>;

  BatchedSender(seastar::lw_shared_ptr<SendingSocketManager>,
                std::chrono::microseconds, std::size_t);

  static seastar::future<>
      resend_on_timer(seastar::lw_shared_ptr<BatchedSender>, std::uint64_t);

  void do_deposit_packet(seastar::net::packet packet);

public:
  BatchedSender(BatchedSender &&) = default;
  BatchedSender &operator=(BatchedSender &&) = default;

  static seastar::lw_shared_ptr<BatchedSender>
      make_batched_sender(seastar::lw_shared_ptr<SendingSocketManager>,
                          std::chrono::microseconds, std::size_t);

  seastar::future<bool> deposit_packet(seastar::net::packet packet);
  seastar::future<> send_batch();

  void add_client(protocol::ClientId id, seastar::shared_ptr<Socket> socket) {
    sockets->add_client(id, std::move(socket));
  }
  bool remove_client(protocol::ClientId id) {
    return sockets->remove_client(id);
  }
  std::vector<SendFailure> get_failed_sockets() {
    return sockets->get_failed_sockets();
  }
};
} // namespace blastoise::net
  // blastoise::net:publicseastar::enable_lw_shared_from_this<TcpSendLoop>