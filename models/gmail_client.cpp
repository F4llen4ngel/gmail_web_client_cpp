#include "gmail_client.h"

#include "cstdlib"
#include "drogon/drogon.h"
#include "drogon/utils/Utilities.h"

Header::Header(const Json::Value &json) {
    name = json["name"].asString();
    value = json["value"].asString();
}

MessagePartBody::MessagePartBody(const Json::Value &json) {
    size = json["size"].asInt();
    data = json["data"].asString();
    attachmentId = json["attachmentId"].asString();
}

MessagePartBody::MessagePartBody() {
    size = 0;
    data = "";
    attachmentId = "";
}

MessagePart::MessagePart(const Json::Value &json) {
    partId = json["partId"].asString();
    mimeType = json["mimeType"].asString();

    for (auto &header: json["headers"]) {
        headers.emplace_back(header);
    }

    if (json["body"].isObject()) {
        body = MessagePartBody(json["body"]);
    } else {
        body = MessagePartBody();
    }

    if (json["parts"].isArray()) {
        for (auto &part: json["parts"]) {
            parts.emplace_back(part);
        }
    }
}

MessagePart::MessagePart() {
    partId = "";
    mimeType = "";
    filename = "";
    headers = std::vector<Header>();
    body = MessagePartBody();
    parts = std::vector<MessagePart>();
}

Message::Message(const Json::Value &json) {
    id = json["id"].asString();
    threadId = json["threadId"].asString();
    snippet = json["snippet"].asString();
    historyId = json["historyId"].asString();
    internalDate = json["internalDate"].asString();
    sizeEstimate = json["sizeEstimate"].asInt();
    raw = json["raw"].asString();

    for (auto &labelId: json["labelIds"]) {
        labelIds.push_back(labelId.asString());
    }

    for (auto &header: json["payload"]["headers"]) {
        payload.headers.emplace_back(header);
    }

    payload.mimeType = json["payload"]["mimeType"].asString();

    if (json["payload"]["body"].isObject()) {
        payload.body = MessagePartBody(json["payload"]["body"]);
    } else {
        payload.body = MessagePartBody();
    }

    if (json["payload"]["parts"].isArray()) {
        for (auto &part: json["payload"]["parts"]) {
            payload.parts.emplace_back(MessagePart(part));
        }
    } else {
        payload.parts = std::vector<MessagePart>();
    }

}

Message::Message() {
    id = "";
    threadId = "";
    labelIds = std::vector<std::string>();
    snippet = "";
    historyId = "";
    internalDate = "";
    sizeEstimate = 0;
    raw = "";
    payload = MessagePart();
}

gmail_client::gmail_client() {
    client_id = std::getenv("CLIENT_ID");
    client_secret = std::getenv("CLIENT_SECRET");
    redirect_uri = std::getenv("REDIRECT_URI");

    if (client_id == nullptr) {
        throw std::runtime_error("CLIENT_ID is not set");
    } else if (client_secret == nullptr) {
        throw std::runtime_error("CLIENT_SECRET is not set");
    } else if (redirect_uri == nullptr) {
        throw std::runtime_error("REDIRECT_URI is not set");
    }

    oauthClient = drogon::HttpClient::newHttpClient("https://oauth2.googleapis.com");
    gmailApiClient = drogon::HttpClient::newHttpClient("https://gmail.googleapis.com");
    apiClient = drogon::HttpClient::newHttpClient("https://www.googleapis.com");

    if (oauthClient == nullptr) {
        throw std::runtime_error("oauthClient is null");
    } else if (gmailApiClient == nullptr) {
        throw std::runtime_error("gmailApiClient is null");
    } else if (apiClient == nullptr) {
        throw std::runtime_error("apiClient is null");
    }

}

std::string gmail_client::getOAuthUrl() {
    std::string baseUrl = "https://accounts.google.com/o/oauth2/v2/auth";
    std::string scope = "https://www.googleapis.com/auth/gmail.readonly";
    std::string response_type = "code";
    std::string access_type = "offline";

    std::string url = baseUrl + "?scope=" + scope + "&client_id=" + client_id + "&redirect_uri=" + redirect_uri +
                      "&response_type=" + response_type + "&access_type=" + access_type;

    return url;
}

void gmail_client::getAccessToken(const std::string &code, Callback &&callback) {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/token");
    req->setParameter("code", code);
    req->setParameter("client_id", client_id);
    req->setParameter("client_secret", client_secret);
    req->setParameter("redirect_uri", redirect_uri);
    req->setParameter("grant_type", "authorization_code");

    oauthClient->sendRequest(req,
                             [callback = std::move(callback)](drogon::ReqResult result,
                                                              const drogon::HttpResponsePtr &response) mutable {
                                 if (result == drogon::ReqResult::Ok) {
                                     drogon::HttpResponsePtr resp = drogon::HttpResponse::newRedirectionResponse("/");

                                     std::string access_token = (*response->getJsonObject())["access_token"].asString();
                                     int expires_in = (*response->getJsonObject())["expires_in"].asInt();

                                     drogon::Cookie cookie("access_token", access_token);
                                     cookie.setExpiresDate(trantor::Date::now().after(expires_in));

                                     resp->addCookie(cookie);

                                     callback(resp);
                                 } else {
                                     LOG_ERROR << "Error: " << result;
                                 }
                             }
    );
}

void gmail_client::getMessages(const std::string &accessToken, Callback &&callback) {
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Get);
    request->setPath("/gmail/v1/users/me/messages");
    request->addHeader("Authorization", "Bearer " + accessToken);

    std::promise<void> listMessagesPromise;
    std::future<void> listMessagesFuture = listMessagesPromise.get_future();

    std::vector<Message> messages;

    gmailApiClient->sendRequest(request,
                                [this, accessToken, &messages, &listMessagesPromise](const drogon::ReqResult &result,
                                                                                     const drogon::HttpResponsePtr &response) mutable {
                                    listMessagesCallback(result, response, accessToken, messages, listMessagesPromise);
                                }
    );

    listMessagesFuture.wait();

    LOG_DEBUG << "listMessagesFuture.get()";

    try {
        listMessagesFuture.get();
    } catch (std::exception &e) {
        LOG_ERROR << e.what();
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        callback(resp);
        return;
    }

    // DEBUG print
    for (auto &message: messages) {
        LOG_DEBUG << "Message Id: " << message.id << " from " << getHeaderValue(message.payload.headers, "From")
                  << " with subject " << getHeaderValue(message.payload.headers, "Subject")
                  << " and date " << getHeaderValue(message.payload.headers, "Date");
    }

    drogon::HttpViewData data;
    data.insert("messages", messages);
    auto resp = drogon::HttpResponse::newHttpViewResponse("gmail.csp", data);
    callback(resp);
}

void gmail_client::listMessagesCallback(const drogon::ReqResult &result, const drogon::HttpResponsePtr &response,
                                        const std::string &accessToken, std::vector<Message> &messages,
                                        std::promise<void> &listMessagesPromise) {
    if (result != drogon::ReqResult::Ok) {
        LOG_ERROR << "Error: " << result;
        listMessagesPromise.set_exception(std::make_exception_ptr(std::runtime_error("Error")));
        return;
    }

    auto messagesJson = (*response->getJsonObject())["messages"];

    std::atomic<int> msgCounter = 0;
    size_t msgCount = messagesJson.size();

    LOG_DEBUG << "messagesJson.size() = " << messagesJson.size();

    for (auto &messageJson: messagesJson) {
        std::string id = messageJson["id"].asString();
        std::string threadId = messageJson["threadId"].asString();

        auto request = drogon::HttpRequest::newHttpRequest();
        request->setMethod(drogon::Get);
        request->setPath("/gmail/v1/users/me/messages/" + id);
        request->addHeader("Authorization", "Bearer " + accessToken);

        LOG_DEBUG << "Sending request for message " << id << " in thread " << threadId << " ...";

        gmailApiClient->sendRequest(request,
                                    [this, &messages, &listMessagesPromise, &msgCounter, &msgCount](
                                            const drogon::ReqResult &result,
                                            const drogon::HttpResponsePtr &response) {

                                        std::string messageId = (*response->getJsonObject())["id"].asString();
                                        LOG_DEBUG << "Message callback is called for message " << messageId;

                                        if (result != drogon::ReqResult::Ok) {
                                            LOG_ERROR << "Error: " << result;
                                            listMessagesPromise.set_exception(
                                                    std::make_exception_ptr(std::runtime_error("Error")));
                                            return;
                                        }

                                        Message msg = Message(*response->getJsonObject());
                                        messages.emplace_back(msg);

                                        msgCounter++;

                                        LOG_DEBUG << "msgCounter = " << msgCounter << " messageJson.size() = "
                                                  << msgCount;

                                        if (msgCounter >= msgCount) {
                                            LOG_DEBUG << "msgCounter == messageJson.size()";
                                            listMessagesPromise.set_value();
                                        }
                                    }
        );
    }
}


std::string gmail_client::getHeaderValue(const std::vector<Header> &headers, const std::string &name) {
    for (auto &header: headers) {
        if (header.name == name) {
            return header.value;
        }
    }

    return "";
}

std::string gmail_client::getBody(const Message &message) {
    if (message.payload.body.size > 0) {
        return drogon::utils::base64Decode(message.payload.body.data);
    }

    if (!message.payload.parts.empty()) {
        for (auto &part: message.payload.parts) {
            if (part.mimeType == "text/html" && part.body.size > 0) {
                return drogon::utils::base64Decode(part.body.data);
            }
        }
        for (auto &part: message.payload.parts) {
            if (part.mimeType == "text/plain" && part.body.size > 0) {
                return drogon::utils::base64Decode(part.body.data);
            }
        }
    }

    return "";
}

void gmail_client::getMessageContent(const std::string &accessToken, const std::string &messageId, Callback &&callback) {
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Get);
    request->setPath("/gmail/v1/users/me/messages/" + messageId);
    request->addHeader("Authorization", "Bearer " + accessToken);

    LOG_DEBUG << "Sending request for message " << messageId << " ...";

    Message *message;
    std::promise<void> messagePromise;
    std::future<void> messageFuture = messagePromise.get_future();

    gmailApiClient->sendRequest(request,
                                [this, &callback, &message, &messagePromise](const drogon::ReqResult &result,
                                                                             const drogon::HttpResponsePtr &response) {
                                    if (result != drogon::ReqResult::Ok) {
                                        LOG_ERROR << "Error: " << result;
                                        messagePromise.set_exception(
                                                std::make_exception_ptr(std::runtime_error("Error")));
                                        auto resp = drogon::HttpResponse::newHttpResponse();
                                        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                                        callback(resp);
                                        return;
                                    }

                                    message = new Message(*response->getJsonObject());
                                    messagePromise.set_value();
                                }
    );

    messageFuture.wait();

    LOG_DEBUG << "messageFuture.get()";

    try {
        messageFuture.get();
    } catch (std::exception &e) {
        LOG_ERROR << e.what();
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        callback(resp);
        return;
    }

    LOG_DEBUG << "Message body: " << drogon::utils::base64Decode(message->payload.body.data);
    LOG_DEBUG << "Message size: " << message->payload.body.size;

    drogon::HttpViewData data;
    std::string msgBody = getBody(*message);
    data.insert("message", *message);
    data.insert("messageBody", msgBody);
    auto resp = drogon::HttpResponse::newHttpViewResponse("message.csp", data);
    callback(resp);
}
