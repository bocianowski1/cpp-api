#ifndef DB
#define DB

#include <pqxx/pqxx>
#include "json.hpp"

using json = nlohmann::json;

json get_transactions(pqxx::connection &C);
json get_transaction_by_id(pqxx::connection &C, int id);
int insert_transaction(pqxx::connection &C, std::string buy_currency, std::string sell_currency, double amount, std::string time);
int delete_transaction(pqxx::connection &C, int id);
pqxx::connection* connect();

#endif // DB
