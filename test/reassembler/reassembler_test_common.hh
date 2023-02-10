#include "reassembler/reassembler.hh"

#include <cstdint>
#include <vector>

using namespace blastoise::reassembler;

inline PacketSequence make_sequence(std::uint32_t sequence) {
  return PacketSequence{.sequence = sequence, .hash = 0};
}
inline Packet make_packet(std::uint32_t sequence,
                          std::vector<std::uint8_t> data) {
  return Packet(make_sequence(sequence), std::move(data));
}

inline ForwardResult forward(Packet p) { return Forward{std::move(p)}; }
inline ForwardResult skip() { return SkipPacket(); }