#pragma once

#include <functional>
#include <memory>
#include <stop_token>

#include "order.hpp"
#include "order_queue.hpp"
#include "thread_safe_queue.hpp"

namespace matching_engine {

using order_queue_t = thread_safe_queue::ThreadSafeQueue<order::Order>;

class MatchingEngine {
public:
  MatchingEngine(std::shared_ptr<order_queue_t> queue) :
    operation_queue_(queue) {}

  void run(std::stop_token stoken);

private:
  void place_order(order::Order order);
  bool cancel_order(order_queue::order_id_t id);

  order_queue::OrderQueue<std::greater<order_queue::order_level_t>> buy_orders_;
  order_queue::OrderQueue<std::less<order_queue::order_level_t>> sell_orders_;
  std::shared_ptr<order_queue_t> operation_queue_;
  // shared pointer to operation queue, and book operation queue 
};

};
