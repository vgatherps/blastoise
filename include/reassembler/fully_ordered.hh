#pragma once

#include "reassembler.hh"

// stl implementation is quite slow but that's irrelevant for a hobby project
// a proper ringbuffer would be better
#include <deque>

namespace blastoise {
namespace reassembler {

template <class Storage> class FullyOrderedReassembler {

  struct PendingPacketBatch {
    std::vector<Packet<Storage>> packets;
  };

  std::deque<PendingPacketBatch> packet_batches;

  ForwardResult<Storage> forward_packet(Packet<Storage> packet);
  void insert_pending(Packet<Storage> packet);

public:
  constexpr static bool HasOrderedPackets = true;
  ForwardResult<Storage> handle_reliable(Packet<Storage> packet,
                                         PacketSequence last_forwarded);

  ForwardResult<Storage> handle_unreliable(Packet<Storage> packet,
                                           PacketSequence last_forwarded,
                                           PacketSequence last_reliable);
};

template <class Storage>
ForwardResult<Storage> FullyOrderedReassembler<Storage>::handle_reliable(
    Packet<Storage> packet, PacketSequence last_forwarded) {

  if (last_forwarded.implies_already_seen(packet.sequence)) {
    return SkipPacket{};
  }

  if (packet.sequence.succeeds(last_forwarded)) [[likely]] {
    return forward_packet(std::move(packet));
  }

  throw std::runtime_error("An out-of-order reliable packet was seen");
}

template <class Storage>
ForwardResult<Storage> FullyOrderedReassembler<Storage>::handle_unreliable(
    Packet<Storage> packet, PacketSequence last_forwarded,
    PacketSequence last_reliable) {

  if (last_forwarded.implies_already_seen(packet.sequence)) {
    return SkipPacket{};
  }

  if (packet.sequence.succeeds(last_forwarded)) {
    return forward_packet(std::move(packet));
  }

  insert_pending(std::move(packet));

  return SkipPacket{};
}

template <class Storage>
ForwardResult<Storage>
FullyOrderedReassembler<Storage>::forward_packet(Packet<Storage> packet) {
  // For simplicity, pending buffers are not merged upon insert
  // but instead upon forwarding. The forwarding code hardly changes,
  // and insertion is far simpler
  PacketSequence forward_tail = packet.sequence;
  PendingPacketBatch to_forward;
  for (; packet_batches.size() > 0; packet_batches.pop_front()) {
    PendingPacketBatch &front_batch = packet_batches.front();

    if (front_batch.packets.empty()) [[unlikely]] {
      throw std::runtime_error(
          "Empty packet buffer seen inside the packet forwarder");
    }

    PacketSequence batch_head_sequence = front_batch.packets.front().sequence;

    if (!batch_head_sequence.ahead_of(forward_tail)) {
      throw std::runtime_error(
          "Head sequence was past or at the start of the pending packets");
    }

    if (!batch_head_sequence.succeeds(forward_tail)) {
      break;
    }

    forward_tail = front_batch.packets.back().sequence;

    if (to_forward.packets.size() == 0) {
      to_forward = std::move(front_batch);
    } else {
      to_forward.packets.insert(
          to_forward.packets.end(),
          std::make_move_iterator(front_batch.packets.begin()),
          std::make_move_iterator(front_batch.packets.end()));
    }
  }

  if (to_forward.packets.empty()) {
    return Forward<Storage>{std::move(packet)};
  } else {
    return BulkForward<Storage>{.head = std::move(packet),
                                .buffered = std::move(to_forward.packets)};
  }
}

template <class Storage>
void FullyOrderedReassembler<Storage>::insert_pending(Packet<Storage> packet) {

  // iterate in reverse order, since we generally expect in order packets
  for (auto pending_batch = packet_batches.rbegin();
       pending_batch != packet_batches.rend(); pending_batch++) {

    if (pending_batch->packets.empty()) [[unlikely]] {
      throw std::runtime_error(
          "Empty packet buffer seen inside the packet forwarder");
    }

    PacketSequence front_sequence = pending_batch->packets.front().sequence;
    PacketSequence back_sequence = pending_batch->packets.back().sequence;

    // See if this packet is in back of the patch
    if (packet.sequence.ahead_of(back_sequence)) {
      if (packet.sequence.succeeds(back_sequence)) {
        pending_batch->packets.emplace_back(std::move(packet));
      } else {
        PendingPacketBatch batch;
        batch.packets.emplace_back(std::move(packet));
        packet_batches.emplace(pending_batch.base(), std::move(batch));
      }
      return;
    }
    // See if this packet is in front of the patch
    else if (front_sequence.ahead_of(packet.sequence)) {
      if (front_sequence.succeeds(packet.sequence)) {

        pending_batch->packets.emplace(pending_batch->packets.begin(),
                                       std::move(packet));
        return;
      }
      // We iterate deeper into the list of packets to look for a spot
    }
    // If it's not after and not before, it's inside so we already have it
    // can freely skip the packet
    else {
      return;
    }
  }

  // We couldn't find any anything pending, meaning that
  // we have something at the front

  PendingPacketBatch batch;
  batch.packets.emplace_back(std::move(packet));
  packet_batches.emplace_front(std::move(batch));
}
} // namespace reassembler
} // namespace blastoise