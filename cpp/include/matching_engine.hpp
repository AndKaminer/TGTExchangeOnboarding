#pragma once

#include <functional>
#include <memory>
#include <stop_token>
#include <variant>

#include "order.hpp"
#include "order_queue.hpp"
#include "thread_safe_queue.hpp"

namespace matching_engine {

using operation_t = std::variant<order::Order, order::Cancel>;
using order_queue_t = thread_safe_queue::ThreadSafeQueue<operation_t>;
using buy_book_t = order_queue::OrderQueue<std::less<order_queue::order_level_t>>;
using sell_book_t = order_queue::OrderQueue<std::greater<order_queue::order_level_t>>;

class MatchingEngine {
public:
  MatchingEngine(std::shared_ptr<order_queue_t> queue) :
    operation_queue_(queue) {}

  void run(std::stop_token stoken);

private:
  void place_order(order::Order order);
  bool cancel_order(order::order_id_t id);

  buy_book_t buy_orders_;
  sell_book_t sell_orders_;
  std::shared_ptr<order_queue_t> operation_queue_;
  // shared pointer to operation queue, and book operation queue 
};

};
