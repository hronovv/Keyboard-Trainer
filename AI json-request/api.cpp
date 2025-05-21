#include "api.h"


size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                     std::string* out) {
    size_t total_size = size * nmemb;
    out->append((char*)contents, total_size);
    return total_size;
}


std::string getResponse(const std::string& userInput) {
    CURL* curl;
    CURLcode res;
    std::string read_buffer;

    curl = curl_easy_init();


    if (curl == nullptr) {
        std::cerr << "Ошибка инициализации cURL." << std::endl;
        return "Ошибка.";
    }

    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://api.groq.com/openai/v1/chat/completions");

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers,
                                "Authorization: Bearer "
                                "gsk_"
                                "dT3oDNrRZFsAXK5jgce8WGdyb3FYPe3504y9HyZtKToSDo"
                                "u109Kh");	// free API (ambiguous)
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string json_data =
        "{\"model\": \"llama-3.3-70b-versatile\", \"messages\": [{\"role\": "
        "\"user\", \"content\": \"" +
        userInput + "\"}]}";
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());


    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    res = curl_easy_perform(curl);


    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
                  << std::endl;
        read_buffer = "Ошибка.";
    } else {
        auto json_response = nlohmann::json::parse(read_buffer);
        std::string response_content =
            json_response["choices"][0]["message"]["content"];
        read_buffer = response_content;
    }


    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return read_buffer;
}
