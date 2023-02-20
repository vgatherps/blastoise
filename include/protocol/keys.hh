#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>

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
//
// A generic ordered stream suffices for when you
// have messages that you don't really care about that much
// A later feature for these would be to drop from the fast path
enum class StreamType {
  Ordered,
  ReceiveAll,
  Depth,
  BBO,
};

struct StreamKey {
  NameKey exchange;
  NameKey market;
  StreamType type;

  bool operator==(const StreamKey &other) const = default;
};

} // namespace blastoise::protocol
namespace std {

using namespace blastoise::protocol;
template <> struct hash<StreamKey> {
  size_t operator()(const StreamKey &k) const {
    uint64_t market_int, exchange_int;
    static_assert(sizeof(exchange_int) == sizeof(k.exchange));
    static_assert(sizeof(market_int) == sizeof(k.market));
    memcpy(&market_int, &k.market, sizeof(market_int));
    memcpy(&exchange_int, &k.exchange, sizeof(exchange_int));
    return hash<uint64_t>()(market_int ^ exchange_int) ^
           hash<uint8_t>()(static_cast<uint8_t>(k.type));
  }
};
} // namespace std