#include "matching_engine.hpp"
#include "order_queue.hpp"

namespace matching_engine {

void MatchingEngine::run() {
  while (true) {
    order::Order new_order {order_queue_->pop()};
    place_order(new_order);
  }
}

void MatchingEngine::place_order(order::Order order) {
  if (order.side == order::OrderSide::BUY) {
    std::optional<order::Order> remaining {sell_orders_.match_order(order)};
    if (remaining.has_value()) {
      buy_orders_.insert_order(remaining.value());
    }
  } else {
    std::optional<order::Order> remaining {buy_orders_.match_order(order)};
    if (remaining.has_value()) {
      sell_orders_.insert_order(remaining.value());
    }
  }
}

bool MatchingEngine::cancel_order(order_queue::order_id_t id) {
  return true;
}

};
