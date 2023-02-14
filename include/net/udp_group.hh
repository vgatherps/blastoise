#pragma once

#include "net/udp_socket.hh"
#include "protocol/client.hh"

#include <seastar/core/pipe.hh>
#include <seastar/core/semaphore.hh>

#include <unordered_map>
#include <vector>

namespace blastoise::net {
class UdpGroup {

  using PacketBatch = std::vector<seastar::future<>>;

  std::unordered_map<protocol::ClientId, UdpSocket> sockets;
  seastar::semaphore outstanding_batch_tracker;
  PacketBatch cached_batch;


public:
  seastar::future<> send_to_all(UdpPacket &packet);
};
} // namespace blastoise::net
