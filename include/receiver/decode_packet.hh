#pragma once

#include "protocol/encoder.hh"
#include "protocol/header.hh"
#include "protocol/keys.hh"
#include "protocol/mask.hh"

#include <seastar/net/packet.hh>

#include <optional>
#include <variant>

namespace blastoise::receiver {

struct SplitPacket {
  protocol::PacketHeader header;
  seastar::net::packet packet_data;
  std::optional<seastar::net::packet> rest;
};

struct NotEnoughBytes {
  seastar::net::packet data;
};

using SplitResult = std::variant<SplitPacket, NotEnoughBytes>;

SplitResult split_packet(seastar::net::packet data);

protocol::StreamKey decode_packet(const protocol::PacketHeader &header,
                                  seastar::net::packet &data,
                                  protocol::PacketMask sender_mask,
                                  protocol::DecoderMap decoders);

} // namespace blastoise::receiver