#include "net/udp_group.hh"

#include <seastar/core/coroutine.hh>
#include <seastar/core/when_all.hh>

namespace blastoise::net {


  seastar::future<> UdpGroup::send_to_all(UdpPacket &packet) {
  // as far as I can tell, you *do not* need to await a seastar future
  // for it to be run to completion? Hence just schedule and ignore
  // the UDP sends since they must be in order

  // If not, need to create some sort of awaiter. for UDP happy to just
  // fire and forget for now, although would want to try and detect
  // whether we have to block (and should throttle). However
  // real limits on small lines come far before machine limits


  co_await this->outstanding_batch_tracker.wait();

  for (auto &[id, socket] : sockets) {
    seastar::future<> sent = socket.send(packet);
    cached_batch.push_back(std::move(sent));
  }

  // this itself takes ownership of the futures. It's too bad it's hard to make an iterator yielding
  // these futures, so there's a somewhat inefficient accumulation inside here
  seastar::future<> await_all = seastar::when_all_succeed(cached_batch.begin(), cached_batch.end());

  cached_batch.clear();

  // TODO ownership problem here - if these futures try to resolve themselves during a shutdown
  // and catch an exception, they'd reference garbage memory.
  // Need to put the semaphore into a shared ptr of sorts
  // or need to relearn enable_shared_from_this?
  seastar::future<> breaks_semaphore = await_all.handle_exception([this] (auto exception) {
    this->outstanding_batch_tracker.broken();
    throw exception;
  });

  seastar::future<> signals_completion = breaks_semaphore.then([this]() {
    this->outstanding_batch_tracker.signal(1);
  });


  co_await std::move(signals_completion);

  co_return;
}

} // namespace blastoise::net
