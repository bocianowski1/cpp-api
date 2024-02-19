#include <curl/curl.h>
#include <iostream>
#include <string>
#include "json.hpp"
#include <tuple>
#include <thread>
#include <vector>

using json = nlohmann::json;
using track = std::pair<std::string, std::string>;

size_t write_callback(void *contents, size_t size, size_t nmemb, std::string *userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string get_spotify_token(CURL *curl, std::string code) {
    CURLcode res;
    std::string readBuffer;

    std::cout << "Getting token" << std::endl;
    if (getenv("ENCODED_SPOTIFY_CREDENTIALS") == nullptr) {
        std::cerr << "ENCODED_SPOTIFY_CREDENTIALS is null" << std::endl;
        return std::string{};
    }

    if (getenv("SERVER_URL") == nullptr) {
        std::cerr << "SERVER_URL is null" << std::endl;
        return std::string{};
    }

    char *postFields;
    asprintf(&postFields, "grant_type=authorization_code&code=%s&redirect_uri=%s/callback", code.c_str(), getenv("SERVER_URL"));

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
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        res = curl_easy_perform(curl);


        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return std::string{};
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    json jsonResponse;
    try {
        jsonResponse = json::parse(readBuffer);
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return std::string{};
    }

    if (jsonResponse.find("error") != jsonResponse.end()) {
        std::cerr << "Spotify API error!\n" << jsonResponse["error_description"] << std::endl;
        return std::string{};
    }

    if (jsonResponse["access_token"] == nullptr) {
        std::cerr << "Access token is null" << std::endl;
        return std::string{};
    }

    std::cout << "Received token\n" << std::endl;

    return jsonResponse["access_token"];
}

std::string get_code(CURL *curl) {
    std::cout << "Getting code. This will take a few seconds..." << std::endl;

    CURLcode res;
    std::string readBuffer;

    char *url;
    asprintf(&url, "%s/code", getenv("SERVER_URL"));

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed in get_code(): " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    if (readBuffer == "") {
        return std::string{};
    }

    json jsonResponse;
    try {
        jsonResponse = json::parse(readBuffer);
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return std::string{};
    }

    if (jsonResponse.find("error") != jsonResponse.end()) {
        std::cerr << "Spotify API error!\n" << jsonResponse["error_description"] << std::endl;
        return std::string{};
    }

    std::cout << "Received code\n" << std::endl;

    return jsonResponse["code"];
}

std::string get_code_with_auth(CURL *curl) {
    std::cout << "Authorizing" << std::endl;

    CURLcode res;

    char *url;
    asprintf(&url, "https://accounts.spotify.com/authorize?client_id=%s&response_type=code&redirect_uri=%s/callback&scope=playlist-modify-public%%20playlist-modify-private", getenv("SPOTIFY_CLIENT_ID"), getenv("SERVER_URL"));

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    free(url);
    return get_code(curl);
}

std::vector<std::pair<std::string, std::string>> get_tracks(CURL *curl, std::string *token, char *playlistId) {
    CURLcode res;
    std::string readBuffer;

    if (playlistId == nullptr) {
        std::cerr << "playlistId is null" << std::endl;
        return json{};
    }

    char *url; 
    asprintf(&url, "https://api.spotify.com/v1/playlists/%s", playlistId);

    struct curl_slist *headers = NULL;
    std::string header = "Authorization: Bearer " + *token;
    headers = curl_slist_append(headers, header.c_str());

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return json{};
        }
    } else {
        std::cerr << "curl_easy_init() failed" << std::endl;
        return json{};
    }

    if (readBuffer == "Not Found") {
        std::cout << "Playlist not Found" << std::endl;
        return json{};
    }

    json jsonResponse;
    try {
        jsonResponse = json::parse(readBuffer);
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return json{};
    }

    if (jsonResponse.find("error") != jsonResponse.end()) {
        std::cerr << "Spotify API error!\n" << jsonResponse["error_description"] << std::endl;
        return json{};
    }

    std::vector<track> tracks;
    for (auto &item : jsonResponse["tracks"]["items"]) {
        std::string id = item["track"]["id"];
        std::string name = item["track"]["name"];
        tracks.push_back({id, name});
    }

    return tracks;
}

bool track_in_playlist(std::vector<track> *tracks, std::string *trackId) {
    for (auto &track : *tracks) {
        if (track.first == *trackId) {
            return true;
        }
    }

    return false;
}

void add_to_playlist(std::string *token, char *playlistId, std::string *trackIdToAdd) {
    CURL *curl = curl_easy_init();
    CURLcode res;
    std::string readBuffer;

    if (playlistId == nullptr || trackIdToAdd == nullptr) {
        std::cerr << "playlistId or trackIdToAdd is null" << std::endl;
        return;
    }

    char *url;
    asprintf(&url, "https://api.spotify.com/v1/playlists/%s/tracks", playlistId);

    struct curl_slist *headers = NULL;
    std::string header = "Authorization: Bearer " + *token;
    headers = curl_slist_append(headers, header.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string body = "{\"uris\": [\"spotify:track:" + *trackIdToAdd + "\"], \"position\": 0}";

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return;
        }
    } else {
        std::cerr << "curl_easy_init() failed" << std::endl;
        return;
    }

    if (readBuffer == "Not Found") {
        std::cout << "Not Found" << std::endl;
        return;
    }

    json jsonResponse;
    try {
        jsonResponse = json::parse(readBuffer);
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return;
    }

    if (jsonResponse.find("error") != jsonResponse.end()) {
        std::cerr << "Spotify API error!\n" << jsonResponse["error_description"] << std::endl;
        return;
    }

    std::cout << "Added " << trackIdToAdd << " to playlist" << std::endl;
    return;
}

int main() {
    std::vector<std::thread> threads;
    CURL *curl;

    std::string code = get_code_with_auth(curl);
    if (code == "") {
        std::cerr << "Error in get_code_with_auth()" << std::endl;
        return 1;
    }

    std::string token = get_spotify_token(&curl, code);
    if (token == "") {
        std::cerr << "Error in get_spotify_token(): " << token << std::endl;
        return 1;
    }

    std::vector<track> onRepeatTracks;
    char *onRepeatPlaylistId = getenv("ON_REPEAT_ID");
    onRepeatTracks = get_tracks(&curl, &token, onRepeatPlaylistId);
    if (onRepeatTracks.size() == 0) {
        std::cerr << "No onRepeatTracks in get_tracks()" << std::endl;
        return 1;
    }

    std::vector<track> wrappedTracks;
    char *wrappedPlaylistId = getenv("ADD_PLAYLIST_ID");
    wrappedTracks = get_tracks(&curl, &token, wrappedPlaylistId);
    if (wrappedTracks.size() == 0) {
        std::cerr << "No wrappedTracks in get_tracks()" << std::endl;
        return 1;
    }

    int i = 1;
    std::vector<track> topTracks;
    for (auto &track : onRepeatTracks) {
        if (!track_in_playlist(&wrappedTracks, &track.first)) {
            topTracks.push_back(track);
        } else {
            std::cout << "| track: " << track.second << " is already in playlist" << std::endl;
            continue;
        }
        
        if (i == 3) {
            break;
        }

        i++;
    }

    if (topTracks.size() == 0) {
        std::cout << "No tracks to add" << std::endl;
        return 0;
    }

    for (auto &track : topTracks) {
        std::cout << "| id: " << track.first << " | name: " << track.second << std::endl;
        threads.push_back(std::thread(&add_to_playlist, &token, wrappedPlaylistId, &track.first));
    }

    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "Success" << std::endl;
    return 0;
}
