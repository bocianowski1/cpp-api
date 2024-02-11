#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <fstream>
#include <map>

std::mutex mu;

void increment(int& val) {
    std::lock_guard<std::mutex> guard(mu);
    val++;
}

int main() {
    std::vector<std::thread> threads;
    std::map<std::string, int> oldDB;
    std::map<std::string, int> newDB;
    std::ifstream infile("db.txt");

    if (!infile.is_open()) {
        return 1;
    }

    std::string line;

    while (std::getline(infile, line)) {
        size_t eqPos = line.find("=");
        if (eqPos == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eqPos);
        std::string strVal = line.substr(eqPos + 1);
        try {
            int val = std::stoi(strVal);
            oldDB[key] = val;
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
    }

    infile.close();

    for (auto& kv : oldDB) {
        std::cout << "before: " << kv.first << " " << kv.second << std::endl;
        for (int i = 0; i < 3; i++) {
            threads.push_back(std::thread(&increment, std::ref(kv.second)));
        }
    }
    
    for (auto& t : threads) {
        t.join();
    }

    std::ofstream outfile("db.txt");

    std::string content;
    for (const auto& kv : oldDB) {
        std::lock_guard<std::mutex> lock(mu);
        content += kv.first + "=" + std::to_string(kv.second) + "\n";
    }

    if (!outfile.is_open()) {
        return 1;
    }

    outfile << content;

    outfile.close();


    std::cout << "\nMain" << std::endl;

    return 0;
}