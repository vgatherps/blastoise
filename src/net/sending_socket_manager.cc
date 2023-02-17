#include "net/sending_socket_manager.hh"

#include <seastar/core/coroutine.hh>
#include <seastar/core/when_all.hh>

namespace blastoise::net {

seastar::future<seastar::future<>>
SendingSocketManager::send_to_all(seastar::net::packet packet) {
  // do_with doesn't like this function for some reason
  // so have to handle it manually
  auto myself = shared_from_this();
  auto units = co_await seastar::get_units(outstanding_batch_tracker, 1);
  auto unit_releaser = [units = std::move(units)]() {};
  co_return do_send_to_all(std::move(myself), std::move(packet))
      .finally(std::move(unit_releaser));
}

seastar::future<> SendingSocketManager::do_send_to_all(
    seastar::lw_shared_ptr<SendingSocketManager> myself,
    seastar::net::packet packet) {
  myself->cached_batch.clear();

  for (auto &[id, socket] : myself->sockets) {

    // callback takes ownership of the socket to ensure it stays alive
    // if an error handler deletes it
    seastar::future<> sent =
        socket->send(packet.share())
            .handle_exception(
                [myself, id, _socket = socket](std::exception_ptr e) {
                  // TODO is it possible for this code to get run before the
                  // event loop starts, given that we never yield?
                  // I rely on this to never modify the map
                  // while iterating over it
                  auto lookup = myself->sockets.find(id);
                  if (lookup != myself->sockets.end()) {
                    myself->sockets.erase(lookup);
                    myself->waiting_failures.push_back(
                        SendFailure{.failed_id = id, .except = std::move(e)});
                  }
                  return seastar::make_ready_future();
                });
    myself->cached_batch.push_back(std::move(sent));
  }

  // this itself takes ownership of the futures. It's too bad it's hard to
  // make an iterator yielding these futures, so there's a somewhat
  // inefficient accumulation inside here
  return seastar::when_all_succeed(myself->cached_batch.begin(),
                                   myself->cached_batch.end());
}

void SendingSocketManager::add_client(protocol::ClientId id,
                                      seastar::shared_ptr<Socket> socket) {
  auto [_, inserted] = sockets.try_emplace(id, std::move(socket));
  if (!inserted) {
    throw std::runtime_error("Duplicate ID insertion");
  }
}

bool SendingSocketManager::remove_client(protocol::ClientId id) {
  return sockets.erase(id);
}

SendingSocketManager::SendingSocketManager(std::size_t max_outstanding)
    : outstanding_batch_tracker(max_outstanding) {
  if (max_outstanding == 0) {
    throw std::runtime_error(
        "A socket manager must allow more than one outstanding udp batch");
  }

  cached_batch.reserve(max_outstanding);
}

seastar::lw_shared_ptr<SendingSocketManager>
SendingSocketManager::make_socket_manager(std::size_t max_outstanding) {
  return seastar::make_lw_shared<SendingSocketManager>(max_outstanding);
}

} // namespace blastoise::net
