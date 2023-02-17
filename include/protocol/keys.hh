#pragma once

#include <array>
#include <cstdint>

namespace blastoise::protocol {

// Stand-in for real symbology
using NameKey = std::array<char, 8>;

static_assert(sizeof(NameKey) == 8, "Bad NameKey size");

// Some notes
// It doesn't make sense to have specific receive-all
// streams, since thay just forwards all messages in the
// order they are received anyways
//
// Similarly, it doesn't make sense to have any generic most-recent
// streams, since there's a fundamental assumption that out-of-order
// behind messages can just be dropped
enum class StreamType {
  Ordered,
  ReceiveAll,
  MostRecent,
  Depth,
  BBO,
};

struct StreamKey {
  NameKey exchange;
  NameKey market;
  StreamType type;
};
} // namespace blastoise::protocol