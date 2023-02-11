#include "reassembler/fully_ordered.hh"
#include "gtest/gtest.h"

#include "reassembler_test_common.hh"

TEST(ForwardsNextUnreliable, FullyOrderedTests) {
  Packet packet = make_packet(113, {1, 2, 3, 4, 5});

  FullyOrderedReassembler<Blob> reass;

  ASSERT_EQ(
      reass.handle_unreliable(packet, make_sequence(112), make_sequence(112)),
      forward(packet));
}

TEST(ForwardsUnreliableOnUnreliable, FullyOrderedTests) {
  Packet next_packet = make_packet(113, {1, 2, 3, 4, 5});
  Packet late_packet = make_packet(114, {6, 7, 8, 9, 10});

  FullyOrderedReassembler<Blob> reass;

  ASSERT_EQ(reass.handle_unreliable(late_packet, make_sequence(112),
                                    make_sequence(112)),
            skip());

  ASSERT_EQ(reass.handle_unreliable(next_packet, make_sequence(112),
                                    make_sequence(112)),
            bulk(next_packet, {late_packet}));
}

TEST(ForwardsWithConsecutiveLate, FullyOrderedTests) {
  Packet next_packet = make_packet(113, {1, 2, 3, 4, 5});
  Packet late_packet = make_packet(114, {6, 7, 8, 9, 10});
  Packet late_packet_2 = make_packet(115, {6, 7, 8, 9, 10});

  FullyOrderedReassembler<Blob> reass;

  ASSERT_EQ(reass.handle_unreliable(late_packet, make_sequence(112),
                                    make_sequence(112)),
            skip());

  ASSERT_EQ(reass.handle_unreliable(late_packet_2, make_sequence(112),
                                    make_sequence(112)),
            skip());

  ASSERT_EQ(reass.handle_unreliable(next_packet, make_sequence(112),
                                    make_sequence(112)),
            bulk(next_packet, {late_packet, late_packet_2}));
}

TEST(ForwardsWithConsecutiveLateOOO, FullyOrderedTests) {
  Packet next_packet = make_packet(113, {1, 2, 3, 4, 5});
  Packet late_packet = make_packet(114, {6, 7, 8, 9, 10});
  Packet late_packet_2 = make_packet(115, {9, 7, 8, 9, 10});

  FullyOrderedReassembler<Blob> reass;

  ASSERT_EQ(reass.handle_unreliable(late_packet_2, make_sequence(112),
                                    make_sequence(112)),
            skip());

  ASSERT_EQ(reass.handle_unreliable(late_packet, make_sequence(112),
                                    make_sequence(112)),
            skip());

  auto expected_bulk = bulk(next_packet, {late_packet, late_packet_2});
  auto found_bulk = reass.handle_unreliable(next_packet, make_sequence(112),
                                            make_sequence(112));
  ASSERT_EQ(found_bulk, expected_bulk);
}

TEST(ForwardsWithNonConsecutiveLateOOO, FullyOrderedTests) {
  Packet next_packet = make_packet(113, {1, 2, 3, 4, 5});
  Packet late_packet = make_packet(114, {6, 7, 8, 9, 10});
  Packet late_packet_2 = make_packet(115, {9, 7, 8, 9, 10});
  Packet late_packet_3 = make_packet(116, {0, 7, 8, 9, 10});

  FullyOrderedReassembler<Blob> reass;

  ASSERT_EQ(reass.handle_unreliable(late_packet_3, make_sequence(112),
                                    make_sequence(112)),
            skip());

  ASSERT_EQ(reass.handle_unreliable(late_packet, make_sequence(112),
                                    make_sequence(112)),
            skip());

  ASSERT_EQ(reass.handle_unreliable(late_packet_2, make_sequence(112),
                                    make_sequence(112)),
            skip());

  auto expected_bulk =
      bulk(next_packet, {late_packet, late_packet_2, late_packet_3});
  auto found_bulk = reass.handle_unreliable(next_packet, make_sequence(112),
                                            make_sequence(112));
  ASSERT_EQ(found_bulk, expected_bulk);
}