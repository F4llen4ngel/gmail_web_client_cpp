//
// Created by Иван Лебедев on 29.09.2023.
//

#include "string"
#include "drogon/drogon.h"

#include "string"
#include "drogon/drogon.h"
#include "../models/gmail_client.h"

typedef std::function<void(const drogon::HttpResponsePtr &)> Callback;

gmail_client client = gmail_client();

/**
 * @brief OAuth redirect handler for Gmail API
 * @param request - request with code parameter
 * @param callback - callback function
 */
void oauthRedirectHandler(const drogon::HttpRequestPtr &request, Callback&& callback) {
    Json::Value jsonBody;
    auto code = request->getParameter("code");

    if (code.empty()) {
        jsonBody["error"] = "code is empty";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(jsonBody);
        callback(resp);
    } else {
        jsonBody["code"] = code;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(jsonBody);
        client.getAccessToken(code, std::move(callback));
    }
}

/**
 * @brief Login page handler
 * @param request - request with code parameter
 * @param callback - callback function
 */
void loginPageHandler(const drogon::HttpRequestPtr &request, Callback&& callback) {
    drogon::HttpViewData data;

    std::string oauthUrl = client.getOAuthUrl();
    data.insert("auth_url", oauthUrl);

    auto resp = drogon::HttpResponse::newHttpViewResponse("login.csp", data);
    callback(resp);
}

/**
 * @brief Index page handler
 * @param request - request with code parameter
 * @param callback - callback function
 */
void rootHandler(const drogon::HttpRequestPtr &request, Callback&& callback) {
    std::string accessToken = request->getCookie("access_token");

    if (accessToken.empty()) {
        auto resp = drogon::HttpResponse::newRedirectionResponse("/login");
        callback(resp);
        return;
    }

    client.getMessages(accessToken, std::move(callback));
}

/**
 * @brief Message page handler
 * @param request - request with code parameter
 * @param callback - callback function
 */
void messageHandler(const drogon::HttpRequestPtr &request, Callback&& callback) {
    std::string access_token = request->getCookie("access_token");
    std::string message_id = request->getParameter("id");

    if (access_token.empty()) {
        auto resp = drogon::HttpResponse::newRedirectionResponse("/login");
        callback(resp);
        return;
    }

    client.getMessageContent(access_token, message_id, std::move(callback));
}