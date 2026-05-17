#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SFML/Graphics.hpp>

// КОНСТАНТЫ РАЗМЕРОВ
const int SIZE = 8;
const int TILE = 64;
const int WIDTH = SIZE * TILE;
const int HEIGHT = SIZE * TILE + 120;

// СМЕЩЕНИЕ ДЛЯ ЦЕНТРИРОВАНИЯ
const int FIELD_WIDTH = SIZE * TILE;
const int FIELD_HEIGHT = SIZE * TILE;
const int OFFSET_X = (WIDTH - FIELD_WIDTH) / 2;
const int OFFSET_Y = (HEIGHT - FIELD_HEIGHT - 20) / 2;

// ЦВЕТА ФИШЕК
const sf::Color COLORS[4] = {
    sf::Color(87, 168, 83),
    sf::Color(102, 255, 255),
    sf::Color(244, 150, 158),
    sf::Color(255, 221, 69)
};

#endif // CONSTANTS_H