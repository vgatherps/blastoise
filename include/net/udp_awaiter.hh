#pragma once

#include <seastar/core/pipe.hh>
#include <seastar/core/future.hh>
#include <seastar/core/semaphore.hh>

#include <optional>

namespace blastoise::net {

  // This class dirves the UDP sends to finality,
  // allowing any exceptions to be raised
  class UdpAwaiter {

    // TODO should really recycle these vectors within a pool
    // Same sort of pool would ideally be used by the UDP packets
    seastar::pipe_reader<PacketBatch> incoming_packet_batches;

  public:

    UdpAwaiter(seastar::pipe_reader<PacketBatch> p);

    std::optional<seastar::future<>> await_next_batch();
  };
}
