#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include <string>
#include <vector>

class Database {
private:
    sqlite3* db;

public:
    Database();
    ~Database();

    void open();
    void save(int score, int time);
    int getBest();
    std::vector<std::pair<int, int>> getTopScores();
    void close();
};

#endif // DATABASE_H