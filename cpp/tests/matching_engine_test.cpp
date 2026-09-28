#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "matching_engine.hpp"
#include "order.hpp"

namespace {

using order::Order;
using order::OrderSide;

constexpr order::order_id_t unknown_id {999'999'999};

// Records every cancel callback invocation, tagged so tests can check that
// each cancel is answered exactly once. The success flag is ignored.
class CallbackRecorder {
public:
  std::function<void(bool)> make(std::string tag) {
    return [this, tag](bool) {
      {
        std::lock_guard<std::mutex> lock {mtx_};
        calls_.push_back(tag);
      }
      cv_.notify_all();
    };
  }

  // Blocks until a callback with the given tag has fired, or the timeout expires.
  bool wait_for(const std::string& tag) {
    std::unique_lock<std::mutex> lock {mtx_};
    return cv_.wait_for(lock, std::chrono::seconds(2), [&] {
      return std::ranges::find(calls_, tag) != calls_.end();
    });
  }

  std::size_t count_for(const std::string& tag) {
    std::lock_guard<std::mutex> lock {mtx_};
    return static_cast<std::size_t>(std::ranges::count(calls_, tag));
  }

private:
  std::mutex mtx_;
  std::condition_variable cv_;
  std::vector<std::string> calls_;
};

class MatchingEngineTest : public ::testing::Test {
protected:
  void SetUp() override {
    engine_thread_ = std::jthread {[this](std::stop_token stoken) { engine_.run(stoken); }};
  }

  void TearDown() override {
    engine_thread_.request_stop();
  }

  Order submit(OrderSide side, unsigned int price, unsigned int quantity) {
    Order order {side, price, quantity};
    queue_->push(order);
    return order;
  }

  void cancel(order::order_id_t id, const std::string& tag) {
    queue_->push(order::Cancel {id, recorder_.make(tag)});
  }

  // The queue is FIFO, so once a trailing cancel has been answered every
  // earlier operation has been fully processed, including any extra callbacks.
  void drain() {
    cancel(unknown_id, "drain");
    ASSERT_TRUE(recorder_.wait_for("drain")) << "engine did not process queue";
  }

  std::shared_ptr<matching_engine::order_queue_t> queue_ {
    std::make_shared<matching_engine::order_queue_t>()
  };
  matching_engine::MatchingEngine engine_ {queue_};
  CallbackRecorder recorder_;
  std::jthread engine_thread_;
};

TEST_F(MatchingEngineTest, CancelRestingSellAnswersOnce) {
  Order resting {submit(OrderSide::SELL, 100, 10)};
  cancel(resting.id, "cancel");
  drain();

  EXPECT_EQ(recorder_.count_for("cancel"), 1u);
}

TEST_F(MatchingEngineTest, CancelRestingBuyAnswersOnce) {
  Order resting {submit(OrderSide::BUY, 100, 10)};
  cancel(resting.id, "cancel");
  drain();

  EXPECT_EQ(recorder_.count_for("cancel"), 1u);
}

TEST_F(MatchingEngineTest, CancelUnknownIdAnswersOnce) {
  cancel(unknown_id, "cancel");
  drain();

  EXPECT_EQ(recorder_.count_for("cancel"), 1u);
}

TEST_F(MatchingEngineTest, CancelTwiceAnswersEachOnce) {
  Order resting {submit(OrderSide::SELL, 100, 10)};
  cancel(resting.id, "first");
  cancel(resting.id, "second");
  drain();

  EXPECT_EQ(recorder_.count_for("first"), 1u);
  EXPECT_EQ(recorder_.count_for("second"), 1u);
}

TEST_F(MatchingEngineTest, CancelFullyFilledOrderAnswersOnce) {
  Order resting {submit(OrderSide::SELL, 100, 10)};
  submit(OrderSide::BUY, 100, 10);
  cancel(resting.id, "cancel");
  drain();

  EXPECT_EQ(recorder_.count_for("cancel"), 1u);
}

TEST_F(MatchingEngineTest, CancelPartiallyFilledOrderAnswersOnce) {
  Order resting {submit(OrderSide::SELL, 100, 10)};
  submit(OrderSide::BUY, 100, 4);
  cancel(resting.id, "cancel");
  drain();

  EXPECT_EQ(recorder_.count_for("cancel"), 1u);
}

}
