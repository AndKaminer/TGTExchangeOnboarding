#pragma once

#include <functional>
#include <memory>

#include "order.hpp"
#include "order_queue.hpp"
#include "thread_safe_queue.hpp"

namespace matching_engine {

class MatchingEngine {
public:
  void run();

private:
  void place_order(order::Order order);
  bool cancel_order(order_queue::order_id_t id);

  order_queue::OrderQueue<std::greater<order_queue::order_level_t>> buy_orders_;
  order_queue::OrderQueue<std::less<order_queue::order_level_t>> sell_orders_;
  std::shared_ptr<thread_safe_queue::ThreadSafeQueue<order::Order>> order_queue_;
  // shared pointer to operation queue, and book operation queue 
};

};
