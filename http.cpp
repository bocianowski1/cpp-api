#include <curl/curl.h>
#include <iostream>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

json fetch() {
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

    return json::parse(readBuffer);
}

int main() {
    json res = fetch();

    std::cout << res["name"] << std::endl;

    return 0;
}
