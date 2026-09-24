#pragma once

#include <algorithm>
#include <cstdint>
#include <queue>
#include <stdexcept>
#include <format>
#include <unordered_map>
#include <list>
#include <optional>
#include <vector>

#include "order.hpp"


namespace order_queue {

using order_level_t = unsigned int;
using order_id_t = std::uintptr_t;
using order_node_t = std::list<order::Order>::iterator;

template <typename comparator>
class OrderQueue {
public:
  std::optional<order::Order> match_order(order::Order order) {
    bool valid_orders_available {
      !order_levels_.empty() and
      !comparator(order_levels_.top(), order.price)
    };
    while (valid_orders_available) {
      order_level_t level {order_levels_.top()};
      // there might be ghost levels from cancellations
      if (level_to_orders_[level].empty()) {
        continue;
      }
      
      bool continue_at_level {
        order.quantity < 0 and
        !level_to_orders_[level].empty()
      };
      
      while (continue_at_level) {
        // we want the front of the list because it's the oldest order
        order_node_t list_front {level_to_orders_[level].begin()};
      
        unsigned int volume {std::min(order.price, list_front->quantity)};
        order.quantity -= volume;
        list_front->quantity -= volume;
      
        if (list_front->quantity == 0) {
          id_to_order_.erase(list_front->id);
          level_to_orders_[level].erase(list_front);
        }

        continue_at_level = order.quantity < 0 and !level_to_orders_[level].empty();
      }

      if (level_to_orders_[level].empty()) {
        order_levels_.pop();        
      }

      if (order.quantity == 0) {
        return std::nullopt;
      }

      valid_orders_available = (
        !order_levels_.empty() and
        !comparator(order_levels_.top(), order.price)
      );

    }

    return order;
  }

  void cancel_order(order_id_t id) {
    if (!id_to_order_.contains(id)) {
      throw std::logic_error(std::format("Cannot cancel order with id: {}", id));
    }

    order_node_t order_node {id_to_order_[id]};
    level_to_orders_[order_node->price].erase(order_node);
    id_to_order_.erase(id);
  }

  void insert_order(order::Order order) {
    order_levels_.push(order.price);
    std::list<order::Order>& level_list {level_to_orders_[order.price]};
    level_list.push_back(order);
    id_to_order_[order.id] = std::prev(level_list.end());
  }
  
private:
  std::priority_queue<order_level_t, std::vector<order_level_t>, comparator> order_levels_ {};
  std::unordered_map<order_level_t, std::list<order::Order>> level_to_orders_ {};
  std::unordered_map<order_id_t, order_node_t> id_to_order_ {};
};

}
