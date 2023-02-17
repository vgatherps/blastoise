#include "protocol/encoder.hh"

#include <array>
#include <stdexcept>

namespace blastoise::protocol::detail {
// Doing the union gives us 100% control over the initialization,
// meaning we can have out cake (directly reference data member)
// and eat it to (get control of load/initialize via static function)
// If this were a normal variable, it would be possible for initialization
// to happen in the wrong order and potentially overwrite set bytes
decltype(decode_byte_map) decode_byte_map = {._ = 0};

namespace {

// This assume static initialization is single threaded
bool initialize_decoder() {
  static bool is_initialized = false;
  if (!is_initialized) {
    is_initialized = true;
    decode_byte_map.decode_byte_map = std::array<DecodeData, 256>();
    return false;
  }

  return true;
}

bool _force_initialization = initialize_decoder();
} // namespace

void do_set_decoder(DecodeData d, std::uint8_t stream_tag) {
  // If called at runtime, we know that the decoder will already have been
  // initialized so it's safe to call the initialization call from multiple
  // threads

  initialize_decoder();

  if (!d.valid()) {
    throw std::runtime_error(
        "Tried to initialize decoder tag with an invalid decoder");
  }

  if (decode_byte_map.decode_byte_map[stream_tag].valid()) {
    throw std::runtime_error("Tried to set an already-set decoder tag");
  }

  decode_byte_map.decode_byte_map[stream_tag] = d;
}
} // namespace blastoise::protocol::detail