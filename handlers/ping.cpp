#include <cstdlib>
#include <drogon/drogon.h>

typedef std::function<void(const drogon::HttpResponsePtr &)> Callback;

/**
 * @brief Ping handler
 * @param request
 * @param callback
 */
void pingHandler(const drogon::HttpRequestPtr &request, Callback&& callback) {
    Json::Value jsonBody;
    jsonBody["message"] = "pong";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(jsonBody);
    callback(resp);
}