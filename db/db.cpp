#include <iostream>
#include <pqxx/pqxx>
#include "json.hpp"
#include "db.h"


using json = nlohmann::json;

json get_transactions(pqxx::connection &C) {
    pqxx::work W{C};
    pqxx::result R{W.exec("SELECT * FROM transactions")};
    W.commit();

    json transactions = json::array();

    for (pqxx::result::const_iterator c = R.begin(); c != R.end(); ++c) {
        json transaction;
        transaction["id"] = c[0].as<int>();
        transaction["buy_currency"] = c[1].as<std::string>();
        transaction["sell_currency"] = c[2].as<std::string>();
        transaction["amount"] = c[3].as<double>();
        transaction["time"] = c[4].as<std::string>();
        transactions.push_back(transaction);
    }

    return transactions;
}

json get_transaction_by_id(pqxx::connection &C, int id) {
    pqxx::work W{C};
    pqxx::result R{W.exec("SELECT * FROM transactions WHERE id = " + std::to_string(id))};
    W.commit();

    json transaction;

    for (pqxx::result::const_iterator c = R.begin(); c != R.end(); ++c) {
        transaction["id"] = c[0].as<int>();
        transaction["buy_currency"] = c[1].as<std::string>();
        transaction["sell_currency"] = c[2].as<std::string>();
        transaction["amount"] = c[3].as<double>();
        transaction["time"] = c[4].as<std::string>();
    }

    return transaction;
}

int insert_transaction(pqxx::connection &C, std::string buy_currency, std::string sell_currency, double amount, std::string time) {
    try {
        pqxx::work W{C};
        pqxx::result R{W.exec("INSERT INTO transactions (buy_currency, sell_currency, amount, time) VALUES ('" + buy_currency + "', '" + sell_currency + "', " + std::to_string(amount) + ", '" + time + "')")};
        W.commit();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

int delete_transaction(pqxx::connection &C, int id) {
    try {
        pqxx::work W{C};
        pqxx::result R{W.exec("DELETE FROM transactions WHERE id = " + std::to_string(id))};
        W.commit();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

pqxx::connection* connect() {
    try {
        pqxx::connection C("dbname=db user=postgres password=postgres host=host.docker.internal port=5432");
        if (C.is_open()) {
            std::cout << "Opened database successfully: " << C.dbname() << std::endl;
        } else {
            std::cout << "Can't open database" << std::endl;
            return;
        }

        pqxx::work W{C};
        pqxx::result R{W.exec("CREATE TABLE IF NOT EXISTS transactions (id serial PRIMARY KEY, buy_currency VARCHAR(10), sell_currency VARCHAR(10), amount DECIMAL, time TIMESTAMP)")};
        W.commit();

        std::cout << "Table created successfully" << std::endl;

        pqxx::work W2{C};
        pqxx::result R2{W2.exec("INSERT INTO transactions (buy_currency, sell_currency, amount, time) VALUES ('USD', 'EUR', 1000, '2024-02-11 00:00:00')")};
        W2.commit();

        std::cout << "Record inserted successfully" << std::endl;

        return &C;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return;
    }

}
