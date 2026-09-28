#pragma once

#include <functional>
#include <uuid/uuid.h>
#include <atomic>

namespace order {

using order_id_t = std::uintptr_t;


enum class OrderSide {
  SELL,
  BUY
};

struct Order {
  Order(OrderSide side_p, unsigned int price_p, unsigned int quantity_p) :
  side{side_p}, price{price_p}, quantity{quantity_p} {
  }

  
  const order_id_t id {++order_counter};
  const OrderSide side {OrderSide::BUY};
  const unsigned int price {0};
  unsigned int quantity {0};

private:
  inline static std::atomic<order_id_t> order_counter {0};

  };

struct Cancel {
  explicit Cancel(order_id_t id, std::function<void(bool)> callback)
  : id_{id}, callback_(callback) {}
  const order_id_t id_;
  const std::function<void(bool)> callback_;
};

};
