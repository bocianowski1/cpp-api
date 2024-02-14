#include <curl/curl.h>
#include <iostream>
#include <string>
#include "json.hpp"
#include <tuple>

using json = nlohmann::json;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

struct SpotifyToken {
    std::string access_token;
    std::string token_type;
    int expires_in;
};

SpotifyToken get_spotify_token(CURL *curl) {
    CURLcode res;
    std::string readBuffer;
    int status;

    char *postFields;
    asprintf(&postFields, "grant_type=client_credentials&client_id=%s&client_secret=%s", getenv("SPOTIFY_CLIENT_ID"), getenv("SPOTIFY_CLIENT_SECRET"));

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return {"", "", 0};
        }

        curl_easy_cleanup(curl);
    }

    auto res_json = json::parse(readBuffer);
    std::cout << "res_json: " << res_json << std::endl;
    return {res_json["access_token"], res_json["token_type"], res_json["expires_in"]};
}


std::pair<json, int> fetch(CURL *curl, const char *url, const char *token) {
    CURLcode res;
    std::string readBuffer;

    struct curl_slist *headers = NULL;
    std::string header = "Authorization: Bearer " + std::string(token);
    headers = curl_slist_append(headers, header.c_str());

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return { json{}, res };
        }
    } else {
        std::cerr << "curl_easy_init() failed" << std::endl;
        return { json{}, 500 };
    }

    if (readBuffer == "Not Found") {
        std::cout << "Not Found" << std::endl;
        return { json{}, 404 };
    }

    json jsonResponse;
    try {
        jsonResponse = json::parse(readBuffer);
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return { json{}, 500 };
    }

    std::cout << "got to the end of fetch" << std::endl;

    if (jsonResponse["error"].is_object()) {
        std::cerr << "Spotify API error: " << jsonResponse["error"]["message"] << std::endl;
        return { jsonResponse, 500 };
    }

    return { jsonResponse, 200 };
}
int main() {
    CURL *curl;
    // SpotifyToken tekenResponse = get_spotify_token();
    // char *token = (char *)tekenResponse.access_token.c_str();
    // std::cout << "token: " << token << std::endl;

    const auto url = "https://api.spotify.com/v1/playlists/37i9dQZF1Epn47BfY5AfsA?limit=5";

    auto [response, status] = fetch(&curl, url, "BQCIN3qM7DI2CItCpaYFeikKabnwajoHaMydAD4anG98OEY0t3LEK8pChtn_PiBaL5XrZuAiyrrhhYy84Mjd3OZx6noDUDL3Gb03-ZftL9EPNUMWaGM");

    if (response.is_null() || response == NULL) {
        std::cerr << "Error: " << status << std::endl;
        return 1;
    }

    switch (status) {
        case 200:
            std::cout << "Success" << std::endl;
            break;
        case 404:
            std::cout << "Not Found" << std::endl;
            break;
        default:
            std::cerr << "Error: " << status << std::endl;
            return 1;
    }

    std::vector<std::pair<std::string, std::string>> tracks;
    for (auto &item : response["tracks"]["items"]) {

        std::string id = item["track"]["id"];
        std::string name = item["track"]["name"];
        tracks.push_back({id, name});
    }

    for (auto &track : tracks) {
        std::cout << "id: " << track.first << " name: " << track.second << std::endl;
    }

    std::cout << "Result (" << response["tracks"]["items"].size() << "): " << std::endl;
}
