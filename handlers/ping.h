//
// Created by Иван Лебедев on 29.09.2023.
//

#ifndef LAAR_TEST_TASK_PING_H
#define LAAR_TEST_TASK_PING_H


#include "drogon/drogon.h"

typedef std::function<void(const drogon::HttpResponsePtr &)> Callback;

void pingHandler(const drogon::HttpRequestPtr &request, Callback&& callback);

#endif //LAAR_TEST_TASK_PING_H