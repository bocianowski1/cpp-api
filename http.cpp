#include <curl/curl.h>
#include <iostream>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int fetch() {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://pokeapi.co/api/v2/pokemon/ditto");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK)
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        else
            std::cout << readBuffer << std::endl;

        curl_easy_cleanup(curl);
    }
    
    return 0;
}

void parse() {
    json j = R"(
        {
            "happy": true,
            "pi": 3.141
        }
    )"_json;

    std::cout << j.dump(4) << std::endl;
}

int main() {
    fetch();
    parse();
    return 0;
}
