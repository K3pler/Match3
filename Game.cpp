#include "Game.h"
#include <iostream>

Game::Game():window(sf::VideoMode(WIDTH, HEIGHT), "Match-3")
    ,score(0)
    ,moves(0)
    ,selectedX(-1), selectedY(-1)
    ,waiting(false)
    ,inGame(false)
    ,showHighScores(false)
    ,startTime(0)
{
    window.setFramerateLimit(60);
    loadResources();
    db.open();
    initBoard();
}

void Game::loadResources() {
    loadBackground();
    loadIcon();
    initTexts();
}

void Game::loadBackground() {
    if (bgTexture.loadFromFile("background.png")) {
        bgSprite.setTexture(bgTexture);
        bgSprite.setScale(
            WIDTH / (float)bgTexture.getSize().x,
            HEIGHT / (float)bgTexture.getSize().y
        );
    }
}

void Game::loadIcon() {
    sf::Image icon;
    if (icon.loadFromFile("icon.png")) {
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    }
}

void Game::initTexts() {
    font.loadFromFile("arial.otf");
    
    title.setFont(font);
    title.setString("MATCH 3");
    title.setCharacterSize(40);
    title.setPosition(WIDTH / 2 - 80, 100);
    title.setFillColor(sf::Color(233, 154, 196));
    
    info.setFont(font);
    info.setString("SPACE - Start | H - Records | ESC - Exit");
    info.setCharacterSize(16);
    info.setPosition(WIDTH / 2 - 180, HEIGHT - 140);
    info.setFillColor(sf::Color::Green);
    
    scoreText.setFont(font);
    scoreText.setCharacterSize(18);
    scoreText.setPosition(10, HEIGHT - 60);
    scoreText.setFillColor(sf::Color::Black);
    
    bestText.setFont(font);
    bestText.setCharacterSize(18);
    bestText.setPosition(WIDTH - 120, HEIGHT - 130);
    bestText.setFillColor(sf::Color(233, 154, 196));
    
    highScoresTitle.setFont(font);
    highScoresTitle.setString("HIGH SCORES");
    highScoresTitle.setCharacterSize(28);
    highScoresTitle.setPosition(WIDTH / 2 - 80, 100);
    highScoresTitle.setFillColor(sf::Color(233, 154, 196));
}

void Game::initBoard() {
    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
            board[y][x] = rand() % 4;
    processAllMatches(board, score);
}

void Game::run() {
    while (window.isOpen()) {
        handleEvents();
        updateGame();
        render();
    }
    db.close();
}

void Game::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
        
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (showHighScores) showHighScores = false;
            else window.close();
        }
        
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::H && !inGame) {
            showHighScores = !showHighScores;
        }
        
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space && !inGame && !showHighScores) {
            initBoard();
            score = 0;
            moves = 0;
            inGame = true;
            waiting = false;
            startTime = time(0);
        }
        
        if (inGame && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            int mouseX = sf::Mouse::getPosition(window).x;
            int mouseY = sf::Mouse::getPosition(window).y;
            int x = (mouseX - OFFSET_X) / TILE;
            int y = (mouseY - OFFSET_Y) / TILE;
            
            if (x >= 0 && x < SIZE && y >= 0 && y < SIZE) {
                if (!waiting) {
                    selectedX = x;
                    selectedY = y;
                    waiting = true;
                } else {
                    if (selectedX == x && selectedY == y) {
                        waiting = false;
                        selectedX = selectedY = -1;
                    } else {
                        bool adjacent = (selectedY == y && abs(selectedX - x) == 1) ||
                                        (selectedX == x && abs(selectedY - y) == 1);
                        if (adjacent) {
                            trySwap(board, selectedX, selectedY, x, y, moves, score);
                        }
                        waiting = false;
                        selectedX = selectedY = -1;
                    }
                }
            }
        }
    }
}

void Game::updateGame() {
    if (moves >= 15 && inGame) {
        inGame = false;
        db.save(score, time(0) - startTime);
    }
}

void Game::render() {
    window.clear();
    
    if (bgTexture.getSize().x > 0) {
        window.draw(bgSprite);
    }
    
    if (showHighScores) {
        drawHighScores();
    } else if (!inGame) {
        drawMenu();
    } else {
        drawGame();
    }
    
    window.display();
}

void Game::drawMenu() {
    window.draw(title);
    window.draw(info);
    bestText.setString("Best: " + std::to_string(db.getBest()));
    window.draw(bestText);
}

void Game::drawHighScores() {
    window.draw(highScoresTitle);
    
    auto topScores = db.getTopScores();
    float y = 150;
    
    sf::Text header1("Score", font, 18);
    header1.setPosition(WIDTH / 2 - 80, y);
    header1.setFillColor(sf::Color::Black);
    window.draw(header1);
    
    sf::Text header2("Time(s)", font, 18);
    header2.setPosition(WIDTH / 2 + 20, y);
    header2.setFillColor(sf::Color::Black);
    window.draw(header2);
    y += 30;
    
    int rank = 1;
    for (const auto& rec : topScores) {
        sf::Text rankText(std::to_string(rank) + ".", font, 18);
        rankText.setPosition(WIDTH / 2 - 110, y);
        rankText.setFillColor(sf::Color::Black);
        window.draw(rankText);
        
        sf::Text scoreTextRec(std::to_string(rec.first), font, 18);
        scoreTextRec.setPosition(WIDTH / 2 - 80, y);
        scoreTextRec.setFillColor(sf::Color::Black);
        window.draw(scoreTextRec);
        
        sf::Text timeText(std::to_string(rec.second), font, 18);
        timeText.setPosition(WIDTH / 2 + 20, y);
        timeText.setFillColor(sf::Color::Black);
        window.draw(timeText);
        
        y += 25;
        rank++;
        if (rank > 5) break;
    }
    
    sf::Text backText("Press ESC to return", font, 16);
    backText.setPosition(WIDTH / 2 - 80, HEIGHT - 150);
    backText.setFillColor(sf::Color::Black);
    window.draw(backText);
}

void Game::drawGame() {
    drawGameField();
    drawSelectedHighlight();
    
    scoreText.setString("Score: " + std::to_string(score) + "  Moves: " + std::to_string(moves) + "/15");
    window.draw(scoreText);
}

void Game::drawGameField() {
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if (board[y][x] >= 0) {
                sf::RectangleShape rect(sf::Vector2f(TILE - 4, TILE - 4));
                rect.setPosition(x * TILE + OFFSET_X + 2, y * TILE + OFFSET_Y + 2);
                rect.setFillColor(COLORS[board[y][x] % 4]);
                rect.setOutlineColor(sf::Color::Transparent);
                rect.setOutlineThickness(2);
                window.draw(rect);
            }
        }
    }
}

void Game::drawSelectedHighlight() {
    if (waiting && selectedX >= 0) {
        sf::RectangleShape hl(sf::Vector2f(TILE, TILE));
        hl.setPosition(selectedX * TILE + OFFSET_X, selectedY * TILE + OFFSET_Y);
        hl.setFillColor(sf::Color::Transparent);
        hl.setOutlineColor(sf::Color::Yellow);
        hl.setOutlineThickness(3);
        window.draw(hl);
    }
}