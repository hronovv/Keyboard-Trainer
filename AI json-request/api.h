#ifndef QT_API_API_H
#define QT_API_API_H
#include <bits/stdc++.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                     std::string* out);
std::string getResponse(const std::string& userInput);
#endif
