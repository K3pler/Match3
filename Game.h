#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include "Constants.h"
#include "Database.h"
#include "GameLogic.h"

class Game {
private:
    sf::RenderWindow window;
    Database db;
    sf::Font font;
    
    // Игровое поле
    int board[SIZE][SIZE];
    
    // Игровые переменные
    int score;
    int moves;
    int selectedX, selectedY;
    bool waiting;
    bool inGame;
    bool showHighScores;
    int startTime;
    
    // Текстовые объекты
    sf::Text title;
    sf::Text info;
    sf::Text scoreText;
    sf::Text bestText;
    sf::Text highScoresTitle;
    
    // Фоновое изображение
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    
    // Вспомогательные методы
    void loadResources();
    void loadBackground();
    void loadIcon();
    void initTexts();
    void initBoard();
    void handleEvents();
    void updateGame();
    void render();
    void drawMenu();
    void drawHighScores();
    void drawGame();
    void drawGameField();
    void drawSelectedHighlight();
    
public:
    Game();
    void run();
};

#endif // GAME_H