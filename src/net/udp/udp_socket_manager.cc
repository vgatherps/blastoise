#include "net/udp/udp_socket_manager.hh"

#include <seastar/core/coroutine.hh>
#include <seastar/core/when_all.hh>

namespace blastoise::net {

seastar::future<> UdpSocketManager::send_to_all(seastar::net::packet packet) {
  return seastar::with_semaphore(
      outstanding_batch_tracker, 1,
      [myself = shared_from_this(), packet = std::move(packet)]() mutable {
        return myself->do_send_to_all(packet);
      });
}

seastar::future<>
UdpSocketManager::do_send_to_all(seastar::net::packet &packet) {
  cached_batch.clear();

  for (auto &[id, socket] : sockets) {

    // callback takes ownership of the socket to ensure it stays alive
    // if an error ahndler deletes it
    seastar::future<> sent =
        socket->send(packet.share())
            .handle_exception([myself = shared_from_this(), id,
                               _socket = socket](std::exception_ptr e) {
              // TODO is it possible for this code to get run before the event
              // loop starts? I rely on this to never modify the map while
              // iterating over it
              auto lookup = myself->sockets.find(id);
              if (lookup != myself->sockets.end()) {
                myself->sockets.erase(lookup);
                myself->waiting_failures.push_back(
                    UdpFailure{.failed_id = id, .except = std::move(e)});
              }
              return seastar::make_ready_future();
            });
    cached_batch.push_back(std::move(sent));
  }

  // this itself takes ownership of the futures. It's too bad it's hard to make
  // an iterator yielding these futures, so there's a somewhat inefficient
  // accumulation inside here
  return seastar::when_all_succeed(cached_batch.begin(), cached_batch.end());
}

UdpSocketManager::UdpSocketManager(std::size_t max_outstanding)
    : outstanding_batch_tracker(max_outstanding) {
  if (max_outstanding == 0) {
    throw std::runtime_error(
        "A socket manager must allow more than one outstanding udp batch");
  }

  cached_batch.reserve(max_outstanding);
}

seastar::lw_shared_ptr<UdpSocketManager>
UdpSocketManager::make_udp_group(std::size_t max_outstanding) {
  return seastar::make_lw_shared<UdpSocketManager>(max_outstanding);
}

} // namespace blastoise::net
