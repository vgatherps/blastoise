#pragma once

#include "net/socket/socket.hh"
#include "protocol/client.hh"

#include <seastar/core/pipe.hh>
#include <seastar/core/semaphore.hh>
#include <seastar/core/shared_ptr.hh>

#include <exception>
#include <unordered_map>
#include <variant>
#include <vector>

namespace blastoise::net {

struct SendFailure {
  protocol::ClientId failed_id;
  std::exception_ptr except;
};

class SendingSocketManager
    : public seastar::enable_lw_shared_from_this<SendingSocketManager> {

  // One could implement some complex protocol
  // to wait for the semaphore to get entirely refilled
  // and then claim all of the outstanding usage
  // to remove from the map
  // it's 99% simpler to just make everything shared
  std::unordered_map<protocol::ClientId, seastar::shared_ptr<Socket>> sockets;
  seastar::semaphore outstanding_batch_tracker;
  std::vector<seastar::future<>> cached_batch;
  std::vector<SendFailure> waiting_failures;

  static seastar::future<>
  do_send_to_all(seastar::lw_shared_ptr<SendingSocketManager>,
                 seastar::net::packet packet);

  friend seastar::enable_lw_shared_from_this<SendingSocketManager>;

  SendingSocketManager(std::size_t max_outstanding);

  friend seastar::lw_shared_ptr<SendingSocketManager>;

public:
  seastar::lw_shared_ptr<SendingSocketManager>
  make_socket_manager(std::size_t max_outstanding);

  seastar::future<seastar::future<>> send_to_all(seastar::net::packet packet);

  std::vector<SendFailure> get_failed_sockets() {
    return std::move(waiting_failures);
  }

  void add_client(protocol::ClientId id, seastar::shared_ptr<Socket> socket);
  bool remove_client(protocol::ClientId id);
};
} // namespace blastoise::net
