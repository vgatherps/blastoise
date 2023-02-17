#pragma once

#include "sender/batched_sender.hh"

#include <vector>

namespace blastoise::net {

class PacketBroadcaster {
  seastar::lw_shared_ptr<BatchedSender> udp_forwarder;
  seastar::lw_shared_ptr<BatchedSender> tcp_forwarder;

  void clear_client(SendFailure fail);

public:
  PacketBroadcaster();

  seastar::future<> broadcast_packet(seastar::net::packet p);
  void check_for_dead_clients();
};

} // namespace blastoise::net