#include "matching_engine.hpp"
#include "order.hpp"
#include <optional>
#include <stop_token>
#include <variant>

namespace matching_engine {

void MatchingEngine::run(std::stop_token stoken) {
  while (!stoken.stop_requested()) {
    std::optional<matching_engine::operation_t> new_operation {operation_queue_->pop(stoken)};
    if (!new_operation) {
      break;
    }
    if (std::holds_alternative<order::Order>(new_operation.value())) {
      place_order(std::get<order::Order>(new_operation.value()));
    } else {
      cancel_order(std::get<order::Cancel>(new_operation.value()));
    }
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

void MatchingEngine::cancel_order(order::Cancel cancel) {
  if (sell_orders_.has_order(cancel.id_)) {
    sell_orders_.cancel_order(cancel.id_);
    cancel.callback_(true);
    return;
  }

  if (buy_orders_.has_order(cancel.id_)) {
    buy_orders_.cancel_order(cancel.id_);
    cancel.callback_(true);
    return;
  }

  cancel.callback_(false);


}

};
