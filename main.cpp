#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sqlite3.h>

using namespace sf;

// РАЗМЕРЫ
const int SIZE = 6;        // поле 6x6 (проще чем 8x8)
const int TILE = 64;       // размер клетки
const int WIDTH = SIZE * TILE;
const int HEIGHT = SIZE * TILE + 80;  // +80 для текста

// КЛАСС БАЗЫ ДАННЫХ
class Database {
private:
    sqlite3* db;
public:
    Database() : db(nullptr) {}

    void open() {
        sqlite3_open("scores.db", &db);
        sqlite3_exec(db, "DROP TABLE IF EXISTS scores;", 0, 0, 0);
        sqlite3_exec(db, "CREATE TABLE scores(score INT, time INT);", 0, 0, 0);
    }

    void save(int score, int time) {
        std::string sql = "INSERT INTO scores VALUES(" + std::to_string(score) + "," + std::to_string(time) + ");";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }

    int getBest() {
        int best = 0;
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "SELECT MAX(score) FROM scores;", -1, &stmt, 0);
        if (sqlite3_step(stmt) == SQLITE_ROW) best = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return best;
    }

    void close() { if (db) sqlite3_close(db); }
};

// ЦВЕТА (4 цвета вместо 6)
Color colors[4] = {Color::Red, Color::Green, Color::Blue, Color::Yellow};

int main() {
    srand(time(0));
    RenderWindow window(VideoMode(WIDTH, HEIGHT), "Match-3");
    window.setFramerateLimit(60);

    Font font;
    font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    Database db;
    db.open();

    // ИГРОВОЕ ПОЛЕ
    int board[SIZE][SIZE];
    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
            board[y][x] = rand() % 4;

    int score = 0;
    int moves = 0;
    int selectedX = -1, selectedY = -1;
    bool waiting = false;
    bool inGame = false;
    int startTime = time(0);

    // ТЕКСТЫ
    Text title("MATCH-3", font, 40);
    title.setPosition(WIDTH/2 - 80, 30);
    title.setFillColor(Color::Yellow);

    Text info("Press SPACE to start", font, 20);
    info.setPosition(WIDTH/2 - 100, HEIGHT - 50);
    info.setFillColor(Color::Green);

    Text scoreText("", font, 20);
    scoreText.setPosition(10, HEIGHT - 40);
    scoreText.setFillColor(Color::White);

    Text bestText("", font, 20);
    bestText.setPosition(WIDTH - 150, HEIGHT - 40);
    bestText.setFillColor(Color::Yellow);

    // ФУНКЦИЯ ПОИСКА КОМБИНАЦИЙ
    auto findMatches = [&](bool match[SIZE][SIZE]) -> bool {
        bool found = false;
        for (int y = 0; y < SIZE; y++)
            for (int x = 0; x < SIZE; x++)
                match[y][x] = false;

        // Горизонтальные
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE - 2; x++) {
                int kind = board[y][x];
                if (kind == -1) continue;
                int len = 1;
                while (x + len < SIZE && board[y][x + len] == kind) len++;
                if (len >= 3) {
                    found = true;
                    for (int k = 0; k < len; k++) match[y][x + k] = true;
                }
                x += len - 1;
            }
        }
        // Вертикальные
        for (int x = 0; x < SIZE; x++) {
            for (int y = 0; y < SIZE - 2; y++) {
                int kind = board[y][x];
                if (kind == -1) continue;
                int len = 1;
                while (y + len < SIZE && board[y + len][x] == kind) len++;
                if (len >= 3) {
                    found = true;
                    for (int k = 0; k < len; k++) match[y + k][x] = true;
                }
                y += len - 1;
            }
        }
        return found;
    };

    // УДАЛЕНИЕ КОМБИНАЦИЙ
    auto removeMatches = [&](bool match[SIZE][SIZE]) {
        for (int y = 0; y < SIZE; y++)
            for (int x = 0; x < SIZE; x++)
                if (match[y][x]) {
                    board[y][x] = -1;
                    score += 10;
                }
    };

    // ГРАВИТАЦИЯ
    auto gravity = [&]() {
        for (int x = 0; x < SIZE; x++) {
            for (int y = SIZE - 1; y >= 0; y--) {
                if (board[y][x] == -1) {
                    for (int up = y - 1; up >= 0; up--) {
                        if (board[up][x] != -1) {
                            board[y][x] = board[up][x];
                            board[up][x] = -1;
                            break;
                        }
                    }
                }
            }
        }
    };

    // ДОБАВЛЕНИЕ НОВЫХ ФИШЕК
    auto refill = [&]() {
        for (int y = 0; y < SIZE; y++)
            for (int x = 0; x < SIZE; x++)
                if (board[y][x] == -1)
                    board[y][x] = rand() % 4;
    };

    // КАСКАДЫ
    auto processMatches = [&]() {
        bool match[SIZE][SIZE];
        while (findMatches(match)) {
            removeMatches(match);
            gravity();
            refill();
        }
    };

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                window.close();

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space && !inGame) {
                // НОВАЯ ИГРА
                for (int y = 0; y < SIZE; y++)
                    for (int x = 0; x < SIZE; x++)
                        board[y][x] = rand() % 4;
                processMatches();
                score = 0;
                moves = 0;
                inGame = true;
                waiting = false;
                startTime = time(0);
            }

            // КЛИК МЫШИ
            if (inGame && event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                int x = Mouse::getPosition(window).x / TILE;
                int y = Mouse::getPosition(window).y / TILE;

                if (x >= 0 && x < SIZE && y >= 0 && y < SIZE) {
                    if (!waiting) {
                        selectedX = x; selectedY = y;
                        waiting = true;
                    } else {
                        bool adjacent = (selectedY == y && abs(selectedX - x) == 1) ||
                                        (selectedX == x && abs(selectedY - y) == 1);
                        if (adjacent) {
                            int temp = board[selectedY][selectedX];
                            board[selectedY][selectedX] = board[y][x];
                            board[y][x] = temp;
                            moves++;

                            int oldScore = score;
                            processMatches();
                            if (score == oldScore) {  // Нет комбинаций - откат
                                temp = board[selectedY][selectedX];
                                board[selectedY][selectedX] = board[y][x];
                                board[y][x] = temp;
                                moves--;
                            }
                        }
                        waiting = false;
                        selectedX = selectedY = -1;
                    }
                }
            }
        }

        // КОНЕЦ ИГРЫ (после 15 ходов)
        if (moves >= 15 && inGame) {
            inGame = false;
            db.save(score, time(0) - startTime);
        }

        // ОТРИСОВКА
        window.clear(Color(20, 20, 40));

        if (!inGame) {
            window.draw(title);
            window.draw(info);
            bestText.setString("Best: " + std::to_string(db.getBest()));
            window.draw(bestText);
        } else {
            // Рисуем поле
            for (int y = 0; y < SIZE; y++) {
                for (int x = 0; x < SIZE; x++) {
                    if (board[y][x] >= 0) {
                        RectangleShape rect(Vector2f(TILE - 4, TILE - 4));
                        rect.setPosition(x * TILE + 2, y * TILE + 2);
                        rect.setFillColor(colors[board[y][x] % 4]);
                        rect.setOutlineColor(Color::Black);
                        rect.setOutlineThickness(2);
                        window.draw(rect);
                    }
                }
            }

            // Рамка выбранной фишки
            if (waiting && selectedX >= 0) {
                RectangleShape hl(Vector2f(TILE, TILE));
                hl.setPosition(selectedX * TILE, selectedY * TILE);
                hl.setFillColor(Color::Transparent);
                hl.setOutlineColor(Color::Yellow);
                hl.setOutlineThickness(3);
                window.draw(hl);
            }

            // Счёт и ходы
            scoreText.setString("Score: " + std::to_string(score) + "  Moves: " + std::to_string(moves) + "/15");
            window.draw(scoreText);
        }

        window.display();
    }

    db.close();
    return 0;
}