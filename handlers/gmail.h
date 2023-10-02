#ifndef LAAR_TEST_TASK_GMAIL_H
#define LAAR_TEST_TASK_GMAIL_H

// https://accounts.google.com/o/oauth2/v2/auth
// ?scope=https://www.googleapis.com/auth/gmail.readonly
// &client_id=YOUR_CLIENT_ID
// &redirect_uri=http://127.0.0.1:8181/redirect
// &response_type=code

#include "string"
#include "drogon/drogon.h"

typedef std::function<void(const drogon::HttpResponsePtr &)> Callback;

void oauthRedirectHandler(const drogon::HttpRequestPtr &request, Callback&& callback);
void loginPageHandler(const drogon::HttpRequestPtr &request, Callback&& callback);
void rootHandler(const drogon::HttpRequestPtr &request, Callback&& callback);
void messageHandler(const drogon::HttpRequestPtr &request, Callback&& callback);

#endif //LAAR_TEST_TASK_GMAIL_H
