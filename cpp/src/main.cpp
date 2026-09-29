#include <drogon/drogon.h>
#include <thread>

#include "api_controller.hpp"
#include "market_data_controller.hpp"
#include "matching_engine.hpp"

int main() {

  std::shared_ptr<matching_engine::order_queue_t> order_queue_ptr {
    std::make_shared<matching_engine::order_queue_t>()
  };

  matching_engine::MatchingEngine engine {order_queue_ptr};

  std::jthread engine_thread(
    [&engine](std::stop_token stoken) {
      engine.run(stoken);
    }
  );
  
  drogon::app().registerController(std::make_shared<api::ApiController>(order_queue_ptr));
  drogon::app().registerController(std::make_shared<api::MarketDataController>());
  drogon::app()
    .addListener("0.0.0.0", 8080)
    .run();
}
