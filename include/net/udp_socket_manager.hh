#pragma once

#include "net/udp_socket.hh"
#include "protocol/client.hh"

#include <seastar/core/pipe.hh>
#include <seastar/core/semaphore.hh>
#include <seastar/core/shared_ptr.hh>

#include <exception>
#include <unordered_map>
#include <variant>
#include <vector>

namespace blastoise::net {

struct UdpFailure {
  protocol::ClientId failed_id;
  std::exception_ptr except;
};

class UdpSocketManager
    : public seastar::enable_lw_shared_from_this<UdpSocketManager> {

  // One could implement some complex protocol
  // to wait for the semaphore to get entirely refilled
  // and then claim all of the outstanding usage
  // to remove from the map
  // it's 99% simpler to just make everything shared
  std::unordered_map<protocol::ClientId, seastar::lw_shared_ptr<UdpSocket>>
      sockets;
  seastar::semaphore outstanding_batch_tracker;
  std::vector<seastar::future<>> cached_batch;
  std::vector<UdpFailure> waiting_failures;

  seastar::future<> do_send_to_all(UdpPacket &packet);

  friend seastar::enable_lw_shared_from_this<UdpSocketManager>;

  UdpSocketManager(std::size_t max_outstanding);

  friend seastar::lw_shared_ptr<UdpSocketManager>;

public:
  seastar::lw_shared_ptr<UdpSocketManager>
  make_udp_group(std::size_t max_outstanding);

  seastar::future<> send_to_all(UdpPacket &packet);

  std::vector<UdpFailure> get_failed_sockets() {
    return std::move(waiting_failures);
  }
};
} // namespace blastoise::net
