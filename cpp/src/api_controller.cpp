#include "api_controller.hpp"

#include <json/value.h>

namespace api {

namespace {

drogon::HttpResponsePtr make_stub_response(const std::string& endpoint) {
  Json::Value body {};
  body["endpoint"] = endpoint;
  body["status"] = "not implemented";

  drogon::HttpResponsePtr response {drogon::HttpResponse::newHttpJsonResponse(body)};
  response->setStatusCode(drogon::k501NotImplemented);
  return response;
}

};

void ApiController::place_order(const drogon::HttpRequestPtr& request, callback_t&& callback) {
  callback(make_stub_response("POST /orders"));
}

void ApiController::cancel_order(const drogon::HttpRequestPtr& request, callback_t&& callback,
                                 order_queue::order_id_t id) {
  callback(make_stub_response(std::format("DELETE /orders/{}", id)));
}

void ApiController::get_book(const drogon::HttpRequestPtr& request, callback_t&& callback) {
  callback(make_stub_response("GET /book"));
}

};
