#include "net/batched_sender.hh"

#include <seastar/core/coroutine.hh>
#include <seastar/core/future.hh>
#include <seastar/core/sleep.hh>
#include <seastar/core/when_all.hh>

#include <coroutine>

namespace blastoise::net {

seastar::future<>
BatchedSender::resend_on_timer(seastar::lw_shared_ptr<BatchedSender> self,
                               std::uint64_t expected_sequence) {
  co_await seastar::sleep(self->resend_time);
  if (self->send_sequence == expected_sequence) {
    // need to verify how seastar timer semantics interact with
    // coroutines but I *should* just be able to ignore this
    // as the timer loop doesn't rewait to send more things
    // this is scheduled by outstanding sends?
    // No point in 'awaiting' something since this future itself is discarded
    // to run sinside the scheduler

    // TODO only do this if nobody is waiting for the arrival
    self->needs_timer_scheduled = true;
    auto send_in_void = self->send_batch();
  }

  // TODO is this needed to ensure scheduling happens?
  co_return;
}

BatchedSender::BatchedSender(
    seastar::lw_shared_ptr<SendingSocketManager> manager,
    std::chrono::microseconds send_timeout, std::size_t max_batch_size)
    : sockets(std::move(manager)), resend_time(send_timeout),
      max_batch_size(max_batch_size), send_sequence(0),
      needs_timer_scheduled(true) {}

seastar::lw_shared_ptr<BatchedSender> BatchedSender::make_batched_sender(
    seastar::lw_shared_ptr<SendingSocketManager> manager,
    std::chrono::microseconds send_timeout, std::size_t max_batch_size) {
  return seastar::make_lw_shared<BatchedSender>(std::move(manager),
                                                send_timeout, max_batch_size);
}

void BatchedSender::do_deposit_packet(seastar::net::packet packet) {

  pending_packets.append_packet(std::move(packet));

  if (needs_timer_scheduled) {
    needs_timer_scheduled = false;

    // Don't wait on the timer loop, just let it run to completion in background
    auto wait_in_void = resend_on_timer(shared_from_this(), send_sequence);
  }
}

seastar::future<bool>
BatchedSender::deposit_packet(seastar::net::packet packet) {

  if (packet.len() > max_batch_size) {
    co_return false;
  }

  if (pending_packets.size() + packet.len() > max_batch_size) {

    // First, send existing batch to make room
    seastar::future<> sent_batch = send_batch();

    // then append our packet
    do_deposit_packet(std::move(packet));

    // Then wait for the sent batch to get scheduled
    co_await std::move(sent_batch);
  } else {
    do_deposit_packet(std::move(packet));
  }
  co_return true;
}

seastar::future<> BatchedSender::send_batch() {
  if (pending_packets.size() > 0) {
    needs_timer_scheduled = true;
    send_sequence += 1;
    return sockets->send_to_all(pending_packets.release_packet())
        .discard_result();
  } else {
    return seastar::make_ready_future();
  }
}

} // namespace blastoise::net