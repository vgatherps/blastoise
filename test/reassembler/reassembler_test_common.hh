#include "reassembler/reassembler.hh"

#include <cstdint>
#include <vector>

using namespace blastoise::reassembler;

using Blob = std::vector<std::uint8_t>;

inline PacketSequence make_sequence(std::uint32_t sequence) {
  return PacketSequence{.sequence = sequence, .hash = 0};
}
inline Packet<Blob> make_packet(std::uint32_t sequence, Blob data) {
  return Packet<Blob>(make_sequence(sequence), std::move(data));
}

inline ForwardResult<Blob> forward(Packet<Blob> p) {
  return Forward<Blob>{std::move(p)};
}

inline ForwardResult<Blob> bulk(Packet<Blob> p,
                                std::vector<Packet<Blob>> rest) {
  return BulkForward<Blob>{.head = std::move(p), .buffered = std::move(rest)};
}
inline ForwardResult<Blob> skip() { return SkipPacket(); }