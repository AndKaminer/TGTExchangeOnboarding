#include "api_controller.hpp"
#include "drogon/HttpResponse.h"
#include "drogon/HttpTypes.h"
#include "order.hpp"

#include <cstdio>
#include <format>
#include <json/value.h>

namespace api {

namespace {

drogon::HttpResponsePtr makeFailedResponse()
{
    Json::Value json;
    json["ok"] = false;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(drogon::k500InternalServerError);
    return resp;
}

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
  auto req_body_ptr {request->jsonObject()};
  if (
    req_body_ptr == nullptr or
    !(*req_body_ptr).isMember("side") or
    !(*req_body_ptr).isMember("price") or
    !(*req_body_ptr).isMember("quantity") or
    !(*req_body_ptr)["price"].isNumeric() or
    !(*req_body_ptr)["quantity"].isNumeric() or
    !(((*req_body_ptr)["side"] == "BUY") or ((*req_body_ptr)["side"] == "SELL"))
  ) {
    callback(makeFailedResponse());
    return;
  }

  order::Order order {
    (*req_body_ptr)["side"] == "BUY" ? order::OrderSide::BUY : order::OrderSide::SELL,
    (*req_body_ptr)["price"].asUInt(),
    (*req_body_ptr)["quantity"].asUInt()
  };

  std::cout << std::format("{}\n", order.id);

  order_queue_ptr_->push(order);

  Json::Value body {};
  body["order_id"] = order.id;
  drogon::HttpResponsePtr response {drogon::HttpResponse::newHttpJsonResponse(body)};
  response->setStatusCode(drogon::k200OK);
  callback(response);
  
}

void ApiController::cancel_order(const drogon::HttpRequestPtr& request, callback_t&& callback,
                                 order_queue::order_id_t id) {
  callback(make_stub_response(std::format("DELETE /orders/{}", id)));
}

void ApiController::get_book(const drogon::HttpRequestPtr& request, callback_t&& callback) {
  callback(make_stub_response("GET /book"));
}

};
