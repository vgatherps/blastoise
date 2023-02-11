#include "reassembler/forward_all.hh"
#include "gtest/gtest.h"

#include "reassembler_test_common.hh"

TEST(ForwardsOneUnreliable, ForwardAllTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  ForwardAllReassembler<Blob> reass;

  ASSERT_EQ(
      reass.handle_unreliable(packet, make_sequence(102), make_sequence(112)),
      forward(packet));
}

TEST(ForwardsOneReliable, ForwardAllTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  ForwardAllReassembler<Blob> reass;

  ASSERT_EQ(reass.handle_reliable(packet, make_sequence(102)), forward(packet));
}

TEST(ForwardsBehindUneliable, ForwardAllTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  ForwardAllReassembler<Blob> reass;

  ASSERT_EQ(
      reass.handle_unreliable(packet, make_sequence(143), make_sequence(112)),
      forward(packet));
}

TEST(ForwardsBehindReliable, ForwardAllTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  ForwardAllReassembler<Blob> reass;

  ASSERT_EQ(reass.handle_reliable(packet, make_sequence(124)), forward(packet));
}

TEST(RejectBehindUneliable, ForwardAllTests) {
  Packet packet = make_packet(123, {1, 2, 3, 4, 5});

  ForwardAllReassembler<Blob> reass;

  ASSERT_EQ(
      reass.handle_unreliable(packet, make_sequence(143), make_sequence(123)),
      skip());
}

TEST(RejectDuplicatePacket, ForwardAllTests) {
  Packet packet = make_packet(124, {1, 2, 3, 4, 5});

  ForwardAllReassembler<Blob> reass;

  ASSERT_EQ(
      reass.handle_unreliable(packet, make_sequence(143), make_sequence(123)),
      forward(packet));
  ASSERT_EQ(
      reass.handle_unreliable(packet, make_sequence(143), make_sequence(123)),
      skip());
}
