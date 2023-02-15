#pragma once

#include "net/packet_accumulator.hh"
#include "net/sending_socket_manager.hh"

#include <seastar/core/shared_ptr.hh>

namespace blastoise::net {
class BatchedSender {
  seastar::lw_shared_ptr<SendingSocketManager> sockets;
  PacketAccumulator pending_packets;

public:
  BatchedSender(seastar::lw_shared_ptr<SendingSocketManager>);
  BatchedSender(BatchedSender &&) = default;
  BatchedSender &operator=(BatchedSender &&) = default;

  void deposit_packet(seastar::net::packet packet);
  std::optional<seastar::future<seastar::future<>>> send_batch();
};
} // namespace blastoise::net
  // blastoise::net:publicseastar::enable_lw_shared_from_this<TcpSendLoop>