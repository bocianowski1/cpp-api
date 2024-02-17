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

std::pair<std::string, int> get_spotify_token(CURL *curl, char *code) {
    CURLcode res;
    std::string readBuffer;
    int status;
    
    char *postFields;
    asprintf(&postFields, "grant_type=authorization_code&code=%s&redirect_uri=http://localhost:3000/callback", code);

    struct curl_slist *headers = NULL;
    std::string header = "Authorization: Basic " + std::string(getenv("ENCODED_SPOTIFY_CREDENTIALS"));
    std::string contentType = "content-type: application/x-www-form-urlencoded";
    headers = curl_slist_append(headers, header.c_str());
    headers = curl_slist_append(headers, contentType.c_str());

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return {std::string{}, 500};
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    json jsonResponse;
    try {
        jsonResponse = json::parse(readBuffer);
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }

    if (jsonResponse["error"].is_object()) {
        std::cerr << "Spotify API error: " << jsonResponse["error"]["message"] << std::endl;
    }

    return {jsonResponse["access_token"], 200};
}

std::pair<std::string, int> get_code(CURL *curl) {
    std::cout << "Getting code" << std::endl;

    std::cout << "spotify client: " << std::string(getenv("SPOTIFY_CLIENT_ID")) << std::endl;

    CURLcode res;
    std::string readBuffer;
    int status;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://go-server:3000/code");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed in get_code(): " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    if (readBuffer == "") {
        return {std::string{}, 500};
    }

    json jsonResponse;
    try {
        jsonResponse = json::parse(readBuffer);
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return {std::string{}, 500};
    }

    if (jsonResponse["error"].is_object()) {
        std::cerr << "Spotify API error: " << jsonResponse["error"]["message"] << std::endl;
        return {std::string{}, 500};
    }

    return {jsonResponse["code"], 200};
}

std::pair<std::string, int> authorize(CURL *curl) {
    std::cout << "Authorizing" << std::endl;

    CURLcode res;
    int status;

    char *url;
    auto redirectUri = "http://localhost:3000/callback";

    asprintf(&url, "https://accounts.spotify.com/authorize?client_id=%s&response_type=code&redirect_uri=%s&scope=playlist-modify-public%%20playlist-modify-private", getenv("SPOTIFY_CLIENT_ID"), redirectUri);

    // "https://accounts.spotify.com/authorize?client_id=89b3a6ef9c30486e9ceb9a386aa57019&response_type=code&redirect_uri=http://localhost:3000/callback&scope=playlist-modify-private%20playlist-modify-public";

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    free(url);
    return get_code(curl);
}

std::pair<json, int> get_on_repeat_tracks(CURL *curl, const char *token) {
    CURLcode res;
    std::string readBuffer;
    const auto *url = "https://api.spotify.com/v1/playlists/37i9dQZF1Epn47BfY5AfsA";

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
        std::cout << "Playlist not Found" << std::endl;
        return { json{}, 404 };
    }

    json jsonResponse;
    try {
        jsonResponse = json::parse(readBuffer);
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return { json{}, 500 };
    }

    if (jsonResponse["error"].is_object()) {
        std::cerr << "Spotify API error: " << jsonResponse["error"]["message"] << std::endl;
        std::cerr << "Response: " << jsonResponse << std::endl;
        return { jsonResponse, 500 };
    }

    return { jsonResponse, 200 };
}

std::pair<json, int> add_to_playlist(CURL *curl, const char *token, std::string *id) {
    CURLcode res;
    std::string readBuffer;
    const auto *url = "https://api.spotify.com/v1/playlists/28Mb7LZeS6AVlgMUPMUO66/tracks";

    struct curl_slist *headers = NULL;
    std::string header = "Authorization: Bearer " + std::string(token);
    headers = curl_slist_append(headers, header.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string body = "{\"uris\": [\"spotify:track:" + *id + "\"], \"position\": 0}";

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

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

    if (jsonResponse["error"].is_object()) {
        std::cerr << "Spotify API error: " << jsonResponse["error"]["message"] << std::endl;
        return { jsonResponse, 500 };
    }

    return { jsonResponse, 200 };
}

int main() {
    system("./env.sh");

    CURL *curl;

    auto [code, cs] = authorize(curl);
    std::cout << "Code: " << code << std::endl;

    if (code == "") {
        std::cerr << "Error in authorize: " << cs << " code: " << code << std::endl;
        return 1;
    }

    auto [tokenStr, ts] = get_spotify_token(&curl, (char *)code.c_str());
    const char *token = tokenStr.c_str();


    auto [response, status] = get_on_repeat_tracks(&curl, token);

    if (response.is_null() || response == NULL) {
        std::cerr << "Error in get: " << status << std::endl;
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

    int i = 0;
    std::vector<std::pair<std::string, std::string>> tracks;
    for (auto &item : response["tracks"]["items"]) {
        i++;

        std::string id = item["track"]["id"];
        std::string name = item["track"]["name"];
        tracks.push_back({id, name});

        if (i == 5) {
            break;
        }
    }

    for (auto &track : tracks) {
        std::cout << "| id: " << track.first << " | name: " << track.second << std::endl;
    }

    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            continue;
        }
        auto [addResponse, addStatus] = add_to_playlist(&curl, token, &tracks[i].first);

        if (addResponse.is_null() || addResponse == NULL) {
            std::cerr << addResponse << std::endl;
            std::cerr << "Error in add: " << addStatus << std::endl;
            return 1;
        }

        switch (addStatus) {
            case 200:
                std::cout << "Success" << std::endl;
                break;
            case 404:
                std::cout << "Not Found" << std::endl;
                break;
            default:
                std::cerr << "Error 500 in add: " << addStatus << std::endl;
                return 1;
        }
    }

    curl_easy_cleanup(curl);
    return 0;
}
