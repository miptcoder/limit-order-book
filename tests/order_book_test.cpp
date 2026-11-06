#include "lob/order_book.hpp"

#include <gtest/gtest.h>

using namespace lob;

namespace {

Order MakeOrder(Id id, Side side, Price price, Volume vol) {
  Order o{};
  o.id = id;
  o.side = side;
  o.price = price;
  o.vol = vol;
  return o;
}

TEST(OrderBookTest, EmptyBook) {
  OrderBook book;
  EXPECT_FALSE(book.bestBid().has_value());
  EXPECT_FALSE(book.bestAsk().has_value());
  EXPECT_EQ(book.orderCount(), 0u);
  EXPECT_EQ(book.volumeAt(Side::Buy, 100), 0u);
  EXPECT_EQ(book.volumeAt(Side::Sell, 100), 0u);
}

TEST(OrderBookTest, BestPrices) {
  OrderBook book;
  ASSERT_TRUE(book.add(MakeOrder(1, Side::Buy, 100, 10)));
  ASSERT_TRUE(book.add(MakeOrder(2, Side::Buy, 101, 10)));
  ASSERT_TRUE(book.add(MakeOrder(3, Side::Sell, 105, 10)));
  ASSERT_TRUE(book.add(MakeOrder(4, Side::Sell, 103, 10)));

  EXPECT_EQ(book.bestBid(), 101);
  EXPECT_EQ(book.bestAsk(), 103);
  EXPECT_EQ(book.orderCount(), 4u);
}

TEST(OrderBookTest, LevelVolumeIsSumOfOrders) {
  OrderBook book;
  ASSERT_TRUE(book.add(MakeOrder(1, Side::Buy, 100, 5)));
  ASSERT_TRUE(book.add(MakeOrder(2, Side::Buy, 100, 7)));

  EXPECT_EQ(book.volumeAt(Side::Buy, 100), 12u);
  EXPECT_EQ(book.volumeAt(Side::Sell, 100), 0u);
  EXPECT_EQ(book.orderCount(), 2u);
}

TEST(OrderBookTest, CancelOneOfTwoKeepsLevel) {
  OrderBook book;
  ASSERT_TRUE(book.add(MakeOrder(1, Side::Buy, 100, 5)));
  ASSERT_TRUE(book.add(MakeOrder(2, Side::Buy, 100, 7)));

  EXPECT_TRUE(book.cancel(1));
  EXPECT_EQ(book.volumeAt(Side::Buy, 100), 7u);
  EXPECT_EQ(book.bestBid(), 100);
  EXPECT_EQ(book.orderCount(), 1u);
}

TEST(OrderBookTest, CancelLastOrderRemovesLevel) {
  OrderBook book;
  ASSERT_TRUE(book.add(MakeOrder(1, Side::Sell, 103, 5)));
  ASSERT_TRUE(book.add(MakeOrder(2, Side::Sell, 104, 5)));

  EXPECT_TRUE(book.cancel(1));
  EXPECT_EQ(book.volumeAt(Side::Sell, 103), 0u);
  EXPECT_EQ(book.bestAsk(), 104);

  EXPECT_TRUE(book.cancel(2));
  EXPECT_FALSE(book.bestAsk().has_value());
  EXPECT_EQ(book.orderCount(), 0u);
}

TEST(OrderBookTest, RejectsInvalidAdd) {
  OrderBook book;
  ASSERT_TRUE(book.add(MakeOrder(1, Side::Buy, 100, 5)));

  EXPECT_FALSE(book.add(MakeOrder(1, Side::Sell, 105, 3)));
  EXPECT_FALSE(book.add(MakeOrder(2, Side::Buy, 100, 0)));

  EXPECT_EQ(book.orderCount(), 1u);
  EXPECT_EQ(book.volumeAt(Side::Buy, 100), 5u);
  EXPECT_FALSE(book.bestAsk().has_value());
}

TEST(OrderBookTest, RejectsInvalidCancel) {
  OrderBook book;
  EXPECT_FALSE(book.cancel(42));

  ASSERT_TRUE(book.add(MakeOrder(1, Side::Buy, 100, 5)));
  EXPECT_TRUE(book.cancel(1));
  EXPECT_FALSE(book.cancel(1));
}

TEST(OrderBookTest, IdCanBeReusedAfterCancel) {
  OrderBook book;
  ASSERT_TRUE(book.add(MakeOrder(1, Side::Buy, 100, 5)));
  ASSERT_TRUE(book.cancel(1));
  EXPECT_TRUE(book.add(MakeOrder(1, Side::Buy, 99, 3)));
  EXPECT_EQ(book.bestBid(), 99);
}

}  // namespace