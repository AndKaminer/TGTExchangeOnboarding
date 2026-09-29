#pragma once

#include <mutex>
#include <string>
#include <unordered_set>

#include <drogon/WebSocketController.h>

namespace api {

class MarketDataController : public drogon::WebSocketController<MarketDataController, false> {
public:
  WS_PATH_LIST_BEGIN
  WS_PATH_ADD("/ws/marketdata", drogon::Get);
  WS_PATH_LIST_END

  void handleNewConnection(const drogon::HttpRequestPtr& request,
                           const drogon::WebSocketConnectionPtr& connection) override;
  void handleNewMessage(const drogon::WebSocketConnectionPtr& connection,
                        std::string&& message,
                        const drogon::WebSocketMessageType& type) override;
  void handleConnectionClosed(const drogon::WebSocketConnectionPtr& connection) override;

  // Sends a message to every connected client. Safe to call from any thread.
  void broadcast(const std::string& message);

private:
  std::mutex connections_mutex_;
  std::unordered_set<drogon::WebSocketConnectionPtr> connections_;
};

};
