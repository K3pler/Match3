#include "Database.h"

Database::Database() : db(nullptr) {}

Database::~Database() {
    close();
}

void Database::open() {
    sqlite3_open("scores.db", &db);
    sqlite3_exec(db, "DROP TABLE IF EXISTS scores;", 0, 0, 0);
    sqlite3_exec(db, "CREATE TABLE scores(score INT, time INT);", 0, 0, 0);
}

void Database::save(int score, int time) {
    std::string sql = "INSERT INTO scores VALUES(" +
                      std::to_string(score) + "," +
                      std::to_string(time) + ");";
    sqlite3_exec(db, sql.c_str(), 0, 0, 0);
}

int Database::getBest() {
    int best = 0;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "SELECT MAX(score) FROM scores;", -1, &stmt, 0);
    if (sqlite3_step(stmt) == SQLITE_ROW) best = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return best;
}

std::vector<std::pair<int, int>> Database::getTopScores() {
    std::vector<std::pair<int, int>> scores;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "SELECT score, time FROM scores ORDER BY score DESC LIMIT 5;", -1, &stmt, 0);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int score = sqlite3_column_int(stmt, 0);
        int time = sqlite3_column_int(stmt, 1);
        scores.push_back({score, time});
    }
    sqlite3_finalize(stmt);
    return scores;
}

void Database::close() {
    if (db) sqlite3_close(db);
}