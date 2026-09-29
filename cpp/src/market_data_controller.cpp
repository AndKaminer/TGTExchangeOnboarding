#include "market_data_controller.hpp"

#include <json/value.h>
#include <json/writer.h>

namespace api {

void MarketDataController::handleNewConnection(const drogon::HttpRequestPtr& request,
                                               const drogon::WebSocketConnectionPtr& connection) {
  std::lock_guard lock {connections_mutex_};
  connections_.insert(connection);
}

void MarketDataController::handleNewMessage(const drogon::WebSocketConnectionPtr& connection,
                                            std::string&& message,
                                            const drogon::WebSocketMessageType& type) {
  return;
}

void MarketDataController::handleConnectionClosed(const drogon::WebSocketConnectionPtr& connection) {
  std::lock_guard lock {connections_mutex_};
  connections_.erase(connection);
}

void MarketDataController::broadcast(const std::string& message) {
  std::lock_guard lock {connections_mutex_};
  for (const auto& connection : connections_) {
    connection->send(message);
  }
}

};
