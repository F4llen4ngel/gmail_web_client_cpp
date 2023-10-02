//
// Created by Иван Лебедев on 29.09.2023.
//

#ifndef LAAR_TEST_TASK_GMAIL_CLIENT_H
#define LAAR_TEST_TASK_GMAIL_CLIENT_H

#include "string"
#include "drogon/drogon.h"

typedef std::function<void(const drogon::HttpResponsePtr &)> Callback;

struct Header {
    std::string name;
    std::string value;

    explicit Header(const Json::Value &json);
};

struct MessagePartBody {
    std::string attachmentId;
    std::string data;
    int size;

    MessagePartBody();
    explicit MessagePartBody(const Json::Value &json);
};

struct MessagePart {
    std::string partId;
    std::string mimeType;
    std::string filename;
    std::vector<Header> headers;
    MessagePartBody body;
    std::vector<MessagePart> parts;

    MessagePart();
    explicit MessagePart(const Json::Value &json);
};

struct Message{
    std::string id;
    std::string threadId;
    std::vector<std::string> labelIds;
    std::string snippet;
    std::string historyId;
    std::string internalDate;
    int sizeEstimate;
    std::string raw;
    MessagePart payload;


    Message();
    explicit Message(const Json::Value &json);
};

class gmail_client {
private:
    const char * client_id;
    const char * client_secret;
    const char * redirect_uri;
    drogon::HttpClientPtr oauthClient;
    drogon::HttpClientPtr gmailApiClient;
    drogon::HttpClientPtr apiClient;

    std::string getHeaderValue(const std::vector<Header> &headers, const std::string &name);
    std::string getBody(const Message &message);

public:
    gmail_client();
    std::string getOAuthUrl();
    void getAccessToken(const std::string& code, Callback&& callback);
    void getMessages(const std::string& accessToken, Callback&& callback);
    void listMessagesCallback(const drogon::ReqResult &result,
                              const drogon::HttpResponsePtr &response,
                              const std::string &accessToken,
                              std::vector<Message> &messages,
                              std::promise<void> &listMessagesPromise);
    void getMessageContent(const std::string &accessToken, const std::string &messageId, Callback &&callback);
};


#endif //LAAR_TEST_TASK_GMAIL_CLIENT_H
