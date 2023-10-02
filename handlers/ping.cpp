#include <cstdlib>
#include <drogon/drogon.h>

typedef std::function<void(const drogon::HttpResponsePtr &)> Callback;

void pingHandler(const drogon::HttpRequestPtr &request, Callback&& callback) {
    Json::Value jsonBody;
    jsonBody["message"] = "pong";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(jsonBody);
    callback(resp);
}