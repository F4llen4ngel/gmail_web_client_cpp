#include <drogon/drogon.h>

#include "handlers/gmail.h"
#include "handlers/ping.h"

int main() {
    drogon::app()
    .registerHandler("/", &rootHandler, {drogon::Get})
    .registerHandler("/login", &loginPageHandler, {drogon::Get})
    .registerHandler("/ping", &pingHandler, {drogon::Get})
    .registerHandler("/redirect", &oauthRedirectHandler, {drogon::Get})
    .registerHandler("/message", &messageHandler, {drogon::Get})
    .loadConfigFile("../config.json")
    .run();
}