#pragma once

#include "protocol/header.hh"
#include "protocol/keys.hh"

#include <concepts>
#include <cstdint>
#include <span>

namespace blastoise::protocol {

struct HashedStreamKey {
  StreamKey key;
  std::uint64_t hash;

  PacketHash compress() const { return hash ^ (hash >> 32); }
};

namespace detail {
template <std::uint64_t N> class IsConstexprInt {};
} // namespace detail

template <class T>
concept Encodeable = requires {
  requires std::same_as<decltype(T::TAG), std::uint64_t> &&
      requires(std::span<std::uint8_t> s) {
    { T::hashed_stream_key_from_bytes(s) } -> std::same_as<HashedStreamKey>;
  } && requires(const T t) {
    { t.hashed_stream_key() } -> std::same_as<HashedStreamKey>;
  } && requires(detail::IsConstexprInt<T::TAG> _) {
    _;
  };
};

using DecodeFn = HashedStreamKey (*)(std::span<std::uint8_t>);

struct DecodeData {
  std::uint64_t tag = 0;
  DecodeFn key_decoder = nullptr;

  bool valid() const { return key_decoder; }
};

template <Encodeable T> constexpr inline DecodeData make_decoder() {
  return DecodeData{.tag = T::TAG,
                    .key_decoder = T::hashed_stream_key_from_bytes};
}

using NetworkTagId = std::uint8_t;

template <class T> using ByteMap = std::span<T, 256>;

using DecoderMap = ByteMap<const DecodeData>;

namespace detail {
union DecoderUnion {
  std::array<DecodeData, 256> decode_byte_map;
  char _;
};
extern DecoderUnion decode_byte_map;

constexpr static std::span<DecodeData, 256> decoders =
    decode_byte_map.decode_byte_map;

void do_set_decoder(DecodeData d, std::uint8_t stream_tag);
} // namespace detail

template <Encodeable E> void set_decoder_for(std::uint8_t stream_tag) {
  do_set_decoder(make_decoder<E>(), stream_tag);
}

constexpr inline DecoderMap prod_decoders() { return detail::decoders; }

} // namespace blastoise::protocol