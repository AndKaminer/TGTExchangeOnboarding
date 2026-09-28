#include <gtest/gtest.h>

#include <optional>
#include <stdexcept>

#include "matching_engine.hpp"
#include "order.hpp"

namespace {

using order::Order;
using order::OrderSide;

Order buy(unsigned int price, unsigned int quantity) {
  return Order {OrderSide::BUY, price, quantity};
}

Order sell(unsigned int price, unsigned int quantity) {
  return Order {OrderSide::SELL, price, quantity};
}

// Incoming buys match against the resting sell book.
class SellBookTest : public ::testing::Test {
protected:
  matching_engine::sell_book_t book_;
};

// Incoming sells match against the resting buy book.
class BuyBookTest : public ::testing::Test {
protected:
  matching_engine::buy_book_t book_;
};

TEST_F(SellBookTest, EmptyBookReturnsOrderUnchanged) {
  Order incoming {buy(100, 10)};
  std::optional<Order> remaining {book_.match_order(incoming)};

  ASSERT_TRUE(remaining.has_value());
  EXPECT_EQ(remaining->id, incoming.id);
  EXPECT_EQ(remaining->quantity, 10u);
}

TEST_F(SellBookTest, BuyBelowBestAskDoesNotMatch) {
  book_.insert_order(sell(100, 10));

  std::optional<Order> remaining {book_.match_order(buy(99, 10))};

  ASSERT_TRUE(remaining.has_value());
  EXPECT_EQ(remaining->quantity, 10u);
}

TEST_F(SellBookTest, ExactFillReturnsNullopt) {
  book_.insert_order(sell(100, 10));

  EXPECT_FALSE(book_.match_order(buy(100, 10)).has_value());
  // Resting order was consumed, so nothing is left to match.
  EXPECT_EQ(book_.match_order(buy(100, 1))->quantity, 1u);
}

TEST_F(SellBookTest, BuyAboveAskMatches) {
  book_.insert_order(sell(100, 10));

  EXPECT_FALSE(book_.match_order(buy(105, 10)).has_value());
}

TEST_F(SellBookTest, PartialFillOfIncomingReturnsRemainder) {
  book_.insert_order(sell(100, 4));

  std::optional<Order> remaining {book_.match_order(buy(100, 10))};

  ASSERT_TRUE(remaining.has_value());
  EXPECT_EQ(remaining->quantity, 6u);
  EXPECT_EQ(remaining->price, 100u);
}

TEST_F(SellBookTest, PartialFillOfRestingLeavesRemainderInBook) {
  book_.insert_order(sell(100, 10));

  EXPECT_FALSE(book_.match_order(buy(100, 3)).has_value());

  std::optional<Order> remaining {book_.match_order(buy(100, 10))};
  ASSERT_TRUE(remaining.has_value());
  EXPECT_EQ(remaining->quantity, 3u);
}

TEST_F(SellBookTest, BestPriceMatchesFirst) {
  book_.insert_order(sell(101, 5));
  book_.insert_order(sell(100, 5));
  book_.insert_order(sell(102, 5));

  // Should take the 100 level, leaving 101 and 102.
  EXPECT_FALSE(book_.match_order(buy(102, 5)).has_value());

  // Nothing left at or below 100.
  EXPECT_EQ(book_.match_order(buy(100, 5))->quantity, 5u);
  // 101 level still available.
  EXPECT_FALSE(book_.match_order(buy(101, 5)).has_value());
}

TEST_F(SellBookTest, SweepsMultipleLevels) {
  book_.insert_order(sell(100, 5));
  book_.insert_order(sell(101, 5));
  book_.insert_order(sell(103, 5));

  std::optional<Order> remaining {book_.match_order(buy(102, 20))};

  ASSERT_TRUE(remaining.has_value());
  EXPECT_EQ(remaining->quantity, 10u);
  // 103 level untouched.
  EXPECT_FALSE(book_.match_order(buy(103, 5)).has_value());
}

TEST_F(SellBookTest, SamePriceMatchesInTimeOrder) {
  Order first {sell(100, 5)};
  Order second {sell(100, 5)};
  book_.insert_order(first);
  book_.insert_order(second);

  EXPECT_FALSE(book_.match_order(buy(100, 5)).has_value());

  // The older order was filled and removed; the newer one is still resting.
  EXPECT_THROW(book_.cancel_order(first.id), std::logic_error);
  EXPECT_NO_THROW(book_.cancel_order(second.id));
}

TEST_F(SellBookTest, CancelledOrderDoesNotMatch) {
  Order resting {sell(100, 10)};
  book_.insert_order(resting);

  book_.cancel_order(resting.id);

  std::optional<Order> remaining {book_.match_order(buy(100, 10))};
  ASSERT_TRUE(remaining.has_value());
  EXPECT_EQ(remaining->quantity, 10u);
}

TEST_F(SellBookTest, CancelOnlyRemovesTargetOrder) {
  Order cancelled {sell(100, 5)};
  Order kept {sell(100, 5)};
  book_.insert_order(cancelled);
  book_.insert_order(kept);

  book_.cancel_order(cancelled.id);

  std::optional<Order> remaining {book_.match_order(buy(100, 10))};
  ASSERT_TRUE(remaining.has_value());
  EXPECT_EQ(remaining->quantity, 5u);
}

TEST_F(SellBookTest, CancelUnknownIdThrows) {
  EXPECT_THROW(book_.cancel_order(12345), std::logic_error);
}

TEST_F(SellBookTest, CancelTwiceThrows) {
  Order resting {sell(100, 10)};
  book_.insert_order(resting);

  book_.cancel_order(resting.id);
  EXPECT_THROW(book_.cancel_order(resting.id), std::logic_error);
}

TEST_F(SellBookTest, LevelCanBeReusedAfterEmptying) {
  book_.insert_order(sell(100, 5));
  EXPECT_FALSE(book_.match_order(buy(100, 5)).has_value());

  book_.insert_order(sell(100, 5));
  EXPECT_FALSE(book_.match_order(buy(100, 5)).has_value());
}

TEST_F(SellBookTest, MatchingSkipsLevelEmptiedByCancel) {
  Order cancelled {sell(100, 5)};
  book_.insert_order(cancelled);
  book_.insert_order(sell(101, 5));
  book_.cancel_order(cancelled.id);

  EXPECT_FALSE(book_.match_order(buy(101, 5)).has_value());
}

TEST_F(BuyBookTest, SellAboveBestBidDoesNotMatch) {
  book_.insert_order(buy(100, 10));

  std::optional<Order> remaining {book_.match_order(sell(101, 10))};

  ASSERT_TRUE(remaining.has_value());
  EXPECT_EQ(remaining->quantity, 10u);
}

TEST_F(BuyBookTest, SellBelowBidMatches) {
  book_.insert_order(buy(100, 10));

  EXPECT_FALSE(book_.match_order(sell(95, 10)).has_value());
}

TEST_F(BuyBookTest, HighestBidMatchesFirst) {
  book_.insert_order(buy(99, 5));
  book_.insert_order(buy(101, 5));
  book_.insert_order(buy(100, 5));

  // Should take the 101 level.
  EXPECT_FALSE(book_.match_order(sell(99, 5)).has_value());

  // Nothing left at or above 101.
  EXPECT_EQ(book_.match_order(sell(101, 5))->quantity, 5u);
  // 100 level still available.
  EXPECT_FALSE(book_.match_order(sell(100, 5)).has_value());
}

TEST_F(BuyBookTest, SweepsMultipleLevels) {
  book_.insert_order(buy(102, 5));
  book_.insert_order(buy(101, 5));
  book_.insert_order(buy(99, 5));

  std::optional<Order> remaining {book_.match_order(sell(100, 20))};

  ASSERT_TRUE(remaining.has_value());
  EXPECT_EQ(remaining->quantity, 10u);
  // 99 level untouched.
  EXPECT_FALSE(book_.match_order(sell(99, 5)).has_value());
}

}
