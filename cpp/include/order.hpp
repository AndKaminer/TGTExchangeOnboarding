#include <cstdint>

namespace order {

enum class OrderSide {
  SELL,
  BUY
};

struct Order {
  unsigned long id {reinterpret_cast<std::uintptr_t>(this)}; // use memory address as unique id
  OrderSide side {OrderSide::BUY};
  unsigned int price {0};
  unsigned int quantity {0};
  };

};
