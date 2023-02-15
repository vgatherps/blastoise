#pragma once

#include "net/batched_sender.hh"

#include <cstdint>

namespace blastoise::net {
seastar::future<> run_timer_loop(seastar::lw_shared_ptr<BatchedSender> sender,
                                 std::size_t send_every_us);
} // namespace blastoise::net