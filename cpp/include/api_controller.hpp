#pragma once

#include <functional>

#include <drogon/HttpController.h>
#include <memory>

#include "matching_engine.hpp"
#include "order.hpp"
#include "order_queue.hpp"
#include "thread_safe_queue.hpp"

namespace api {


using callback_t = std::function<void(const drogon::HttpResponsePtr&)>;

class ApiController : public drogon::HttpController<ApiController, false> {
public:
  explicit ApiController(std::shared_ptr<matching_engine::order_queue_t> qptr) :
    order_queue_ptr_{qptr} {}

  METHOD_LIST_BEGIN
  ADD_METHOD_TO(ApiController::place_order, "/orders", drogon::Post);
  ADD_METHOD_TO(ApiController::cancel_order, "/orders/{id}", drogon::Delete);
  ADD_METHOD_TO(ApiController::get_book, "/book", drogon::Get);
  METHOD_LIST_END

  void place_order(const drogon::HttpRequestPtr& request, callback_t&& callback);
  void cancel_order(const drogon::HttpRequestPtr& request, callback_t&& callback,
                    order_queue::order_id_t id);
  void get_book(const drogon::HttpRequestPtr& request, callback_t&& callback);

private:
  std::shared_ptr<matching_engine::order_queue_t> order_queue_ptr_;
};

};
