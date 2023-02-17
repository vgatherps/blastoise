#pragma once

#include "protocol/keys.hh"

#include <concepts>
#include <cstdint>
#include <span>

namespace blastoise::protocol {

namespace detail {
template <std::uint64_t N> class IsConstexprInt {};
} // namespace detail

template <class T>
concept Encodeable = requires {
  requires std::same_as<decltype(T::TAG), std::uint64_t> &&
      requires(std::span<std::uint8_t> s) {
    { T::stream_key_from_bytes(s) } -> std::same_as<StreamKey>;
  } && requires(const T t) {
    { t.stream_key() } -> std::same_as<StreamKey>;
  } && requires(detail::IsConstexprInt<T::TAG> _) {
    _;
  };
};

using DecodeFn = StreamKey (*)(std::span<std::uint8_t>);

struct DecodeData {
  std::uint64_t tag = 0;
  DecodeFn key_decoder = nullptr;

  bool valid() const { return key_decoder; }
};

template <Encodeable T> constexpr inline DecodeData make_decoder() {
  return DecodeData{.tag = T::TAG, .key_decoder = T::stream_key_from_bytes};
}

using NetworkTagId = std::uint8_t;

namespace detail {
extern union {
  std::array<DecodeData, 256> decode_byte_map;
  char _;
} decode_byte_map;
void do_set_decoder(DecodeData d, std::uint8_t stream_tag);
} // namespace detail

DecodeData load_decoder_for(NetworkTagId);
template <Encodeable E> void set_decoder_for(std::uint8_t stream_tag) {
  do_set_decoder(make_decoder<E>(), stream_tag);
}

} // namespace blastoise::protocol