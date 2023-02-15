#pragma once

#include "net/packet_accumulator.hh"
#include "net/sending_socket_manager.hh"

#include <seastar/core/shared_ptr.hh>

namespace blastoise::net {
class BatchedSender
    : public seastar::enable_lw_shared_from_this<BatchedSender> {
  SendingSocketManager sockets;
  PacketAccumulator pending_packets;

public:
  BatchedSender(SendingSocketManager);
  BatchedSender(BatchedSender &&) = default;
  BatchedSender &operator=(BatchedSender &&) = default;

  void deposit_packet(seastar::net::packet packet);
  std::optional<seastar::future<seastar::future<>>> send_batch();
};
} // namespace blastoise::net
  // blastoise::net:publicseastar::enable_lw_shared_from_this<TcpSendLoop>