#include "net/run_timer_loop.hh"

#include <seastar/core/sleep.hh>
#include <seastar/core/when_all.hh>

#include <chrono>

namespace blastoise::net {

// TODO this timer now runs independently of the standard packet sending
// instead, it actually needs to reset
seastar::future<> run_timer_loop(seastar::lw_shared_ptr<BatchedSender> sender,
                                 std::size_t send_every_us) {
  auto send_timer = std::chrono::microseconds(send_every_us);
  return seastar::keep_doing([sender, send_timer]() {
    auto is_sending = sender->send_batch();
    seastar::future<> timer = seastar::sleep(send_timer);

    if (is_sending.has_value()) {
      return seastar::when_all_succeed(is_sending->discard_result(),
                                       std::move(timer))
          .discard_result();
    } else {
      return timer;
    }
  });
}
} // namespace blastoise::net