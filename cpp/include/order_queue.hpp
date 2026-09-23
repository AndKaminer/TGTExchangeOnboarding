#include <cstdint>
#include <optional>
#include <queue>
#include <unordered_map>
#include <list>

#include <order.hpp>
#include <vector>

namespace order_queue {

using order_level_t = unsigned int;
using order_id_t = std::uintptr_t;

template <typename comparator>
class OrderQueue {
public:
  bool match_order(order::Order order);
  bool cancel_order(order_id_t id);
  

private:
  std::priority_queue<order_level_t, std::vector<order_level_t>, comparator> order_levels_ {};
  std::unordered_map<order_level_t, std::list<order::Order>> level_to_orders_ {};
  std::unordered_map<order_id_t, std::list<order::Order>::iterator> id_to_order_ {};
};

}
