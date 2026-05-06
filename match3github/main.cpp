
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include "sqlite3.h"

using namespace sf;


// КОНСТАНТЫ РАЗМЕРОВ

const int SIZE = 8;              // Размер поля 8x8
const int TILE = 64;             // Размер одной клетки (пикселей)
const int WIDTH = SIZE * TILE;   // Ширина окна
const int HEIGHT = SIZE * TILE + 120;  // Высота окна (+120 для текста)


// КЛАСС БАЗЫ ДАННЫХ

class Database {
private:
    sqlite3* db;  // Указатель на базу данных

public:
    Database() : db(nullptr) {}           // Конструктор
    void open();                          // Открыть БД и создать таблицу
    void save(int score, int time);       // Сохранить рекорд
    int getBest();                        // Получить лучший рекорд
    std::vector<std::pair<int, int>> getTopScores();  // Получить топ-5
    void close();                         // Закрыть БД
};

// Реализация методов Database
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

void Database::close() { if (db) sqlite3_close(db); }


// ЦВЕТА ФИШЕК

Color colors[4] =
    {Color (87,168,83),
    Color (102,255,255),
    Color (244,150,158),
    Color (255,221,69)};


// ФУНКЦИЯ 1: ПОИСК КОМБИНАЦИЙ

// Возвращает: true, если есть хотя бы одна комбинация
// Алгоритм:
//   1. Ищем горизонтальные комбинации
//   2. Ищем вертикальные комбинации
//   3. Отмечаем match[y][x] = true для фишек, входящих в комбинации
bool findMatches(int board[SIZE][SIZE], bool match[SIZE][SIZE]) {
    bool found = false;


    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
            match[y][x] = false;

    // Горизонтальные комбинации
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

    // Вертикальные комбинации
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
}


// ФУНКЦИЯ 2: УДАЛЕНИЕ КОМБИНАЦИЙ И НАЧИСЛЕНИЕ ОЧКОВ

// Действие:
//   1. Заменяем отмеченные фишки на -1 (пустота)
//   2. Начисляем +11 очков за каждую удалённую фишку
void removeMatches(int board[SIZE][SIZE], bool match[SIZE][SIZE], int& score) {
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if (match[y][x]) {
                board[y][x] = -1;   // -1 означает "пустая клетка"
                score += 11;         // +11 очков
            }
        }
    }
}


// ФУНКЦИЯ 3: ГРАВИТАЦИЯ

// Алгоритм:
//   1. Проходим по каждому столбцу снизу вверх
//   2. Если находим пустую клетку (-1), ищем сверху непустую
//   3. Перемещаем найденную фишку вниз
void applyGravity(int board[SIZE][SIZE]) {
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
}


// ФУНКЦИЯ 4: ДОБАВЛЕНИЕ НОВЫХ ФИШЕК

// Действие: Заменяет все пустые клетки (-1) на случайные фишки (0-3)
void addNewPieces(int board[SIZE][SIZE]) {
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if (board[y][x] == -1) {
                board[y][x] = rand() % 4;  // Случайный цвет 0-3
            }
        }
    }
}


// ФУНКЦИЯ 5: КАСКАДНАЯ ОБРАБОТКА

// Действие: Повторяет цикл, пока есть комбинации:
//   1. Найти комбинации  2. Удалить и начислить очки
//   3. Применить гравитацию  4. Добавить новые фишки
void processAllMatches(int board[SIZE][SIZE], int& score) {
    bool match[SIZE][SIZE];
    while (findMatches(board, match)) {
        removeMatches(board, match, score);
        applyGravity(board);
        addNewPieces(board);
    }
}


// ФУНКЦИЯ 6: ОБМЕН ФИШЕК (с проверкой и откатом)

// Возвращает: true, если были комбинации
bool trySwap(int board[SIZE][SIZE], int x1, int y1, int x2, int y2, int& moves, int& score) {
    // Проверка: фишки соседние?
    if (abs(x1 - x2) + abs(y1 - y2) != 1) return false;

    // Обмен
    int temp = board[y1][x1];
    board[y1][x1] = board[y2][x2];
    board[y2][x2] = temp;

    // Запоминаем старый счёт
    int oldScore = score;

    // Проверяем комбинации
    processAllMatches(board, score);

    // Если счёт не изменился - комбинаций не было
    if (score == oldScore) {
        // Откат обмена
        temp = board[y1][x1];
        board[y1][x1] = board[y2][x2];
        board[y2][x2] = temp;
        return false;
    }

    moves++;  // Увеличиваем счётчик ходов
    return true;
}


// ГЛАВНАЯ ФУНКЦИЯ

int main() {

    // 1. НАЧАЛЬНАЯ НАСТРОЙКА

    srand(time(0)); // Инициализация ГСЧ

    RenderWindow window(VideoMode(WIDTH, HEIGHT), "Match-3");
    window.setFramerateLimit(60);

    Font font;
    font.loadFromFile("arial.otf"); // Загрузка шрифта

    Database db;
    db.open();  // Открытие БД


    // 2. СОЗДАНИЕ ИГРОВОГО ПОЛЯ

    int board[SIZE][SIZE];
    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
            board[y][x] = rand() % 4;


    // 3. ИГРОВЫЕ ПЕРЕМЕННЫЕ

    int score = 0;                           // Текущий счёт
    int moves = 0;                           // Счётчик ходов
    int selectedX = -1, selectedY = -1;      // Выбранная фишка
    bool waiting = false;                    // Ожидание второй фишки
    bool inGame = false;                     // true = игра, false = меню
    bool showHighScores = false;             // Показ таблицы рекордов
    int startTime = time(0);                 // Время начала игры

    auto topScores = db.getTopScores();      // Топ-5 рекордов из БД


    // 4. ТЕКСТОВЫЕ ОБЪЕКТЫ SFML

    Text title("MATCH-3", font, 40);
    title.setPosition(WIDTH/2 - 80, 20);
    title.setFillColor(Color::Yellow);

    Text info("SPACE - Start | H - Records | ESC - Exit", font, 16);
    info.setPosition(WIDTH/2 - 180, HEIGHT - 30);
    info.setFillColor(Color::Green);

    Text scoreText("", font, 18);
    scoreText.setPosition(10, HEIGHT - 60);
    scoreText.setFillColor(Color::White);

    Text bestText("", font, 18);
    bestText.setPosition(WIDTH - 120, HEIGHT - 60);
    bestText.setFillColor(Color::Yellow);

    Text highScoresTitle("HIGH SCORES", font, 28);
    highScoresTitle.setPosition(WIDTH/2 - 90, 50);
    highScoresTitle.setFillColor(Color::Yellow);


    // 5. ГЛАВНЫЙ ИГРОВОЙ ЦИКЛ

    while (window.isOpen()) {


        // 5.1 ОБРАБОТКА СОБЫТИЙ (клавиатура, мышь)

        Event event;
        while (window.pollEvent(event)) {

            // Закрытие окна
            if (event.type == Event::Closed)
                window.close();

            // Клавиша ESC
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                if (showHighScores) showHighScores = false;
                else window.close();
            }

            // Клавиша H (таблица рекордов)
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::H && !inGame) {
                showHighScores = !showHighScores;
                if (showHighScores) topScores = db.getTopScores();
            }

            // Клавиша SPACE (начать игру)
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space && !inGame && !showHighScores) {
                // Создаём новое поле
                for (int y = 0; y < SIZE; y++)
                    for (int x = 0; x < SIZE; x++)
                        board[y][x] = rand() % 4;
                processAllMatches(board, score);  // Убираем начальные комбинации
                score = 0;
                moves = 0;
                inGame = true;
                waiting = false;
                startTime = time(0);
            }

            // Клик мыши
            if (inGame && event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                int x = Mouse::getPosition(window).x / TILE;
                int y = Mouse::getPosition(window).y / TILE;

                if (x >= 0 && x < SIZE && y >= 0 && y < SIZE) {
                    if (!waiting) {
                        // Первый клик: выбираем фишку
                        selectedX = x;
                        selectedY = y;
                        waiting = true;
                    } else {
                        // Второй клик: пробуем обменять
                        if (selectedX == x && selectedY == y) {
                            // Кликнули на ту же фишку — сброс выбора
                            waiting = false;
                            selectedX = selectedY = -1;
                        } else {
                            bool adjacent = (selectedY == y && abs(selectedX - x) == 1) ||
                                            (selectedX == x && abs(selectedY - y) == 1);
                            if (adjacent) {
                                // Пробуем обменять фишки
                                if (trySwap(board, selectedX, selectedY, x, y, moves, score)) {
                                    // Обмен успешен
                                }
                            }
                            waiting = false;
                            selectedX = selectedY = -1;
                        }
                    }
                }
            }
        }


        // 5.2 ПРОВЕРКА ОКОНЧАНИЯ ИГРЫ (после 15 ходов)

        if (moves >= 15 && inGame) {
            inGame = false;
            db.save(score, time(0) - startTime);
            topScores = db.getTopScores();
        }


        // 5.3 ОТРИСОВКА (рендеринг)

        window.clear(Color(20, 20, 40));

        if (showHighScores) {
            //ТАБЛИЦА РЕКОРДОВ
            window.draw(highScoresTitle);

            float y = 100;
            Text header1("Score", font, 18);
            header1.setPosition(WIDTH/2 - 80, y);
            header1.setFillColor(Color::Yellow);
            window.draw(header1);

            Text header2("Time(s)", font, 18);
            header2.setPosition(WIDTH/2 + 20, y);
            header2.setFillColor(Color::Yellow);
            window.draw(header2);
            y += 30;

            int rank = 1;
            for (const auto& rec : topScores) {
                Text rankText(std::to_string(rank) + ".", font, 18);
                rankText.setPosition(WIDTH/2 - 110, y);
                rankText.setFillColor(Color::White);
                window.draw(rankText);

                Text scoreTextRec(std::to_string(rec.first), font, 18);
                scoreTextRec.setPosition(WIDTH/2 - 80, y);
                scoreTextRec.setFillColor(Color::White);
                window.draw(scoreTextRec);

                Text timeText(std::to_string(rec.second), font, 18);
                timeText.setPosition(WIDTH/2 + 20, y);
                timeText.setFillColor(Color::White);
                window.draw(timeText);

                y += 25;
                rank++;
                if (rank > 5) break;
            }

            Text backText("Press ESC to return", font, 16);
            backText.setPosition(WIDTH/2 - 100, HEIGHT - 50);
            backText.setFillColor(Color(150,150,150));
            window.draw(backText);

        } else if (!inGame) {
            //МЕНЮ
            window.draw(title);
            window.draw(info);
            bestText.setString("Best: " + std::to_string(db.getBest()));
            window.draw(bestText);
        } else {
            //ИГРА: ОТРИСОВКА ПОЛЯ
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

        window.display();  // Показать всё на экране
    }


    // 6. ЗАВЕРШЕНИЕ РАБОТЫ
    db.close();
    return 0;
}