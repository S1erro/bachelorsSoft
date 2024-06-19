#ifndef DISPLAY_H
#define DISPLAY_H

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <fstream> // ��� std::ofstream

class Display {
public:
    Display(int width, int height, const std::string& filename);
    void clear();
    void drawText(int x, int y, const std::string& text);
    void draw(sf::RenderWindow& window);
    void saveToFile();

private:
    int width, height;
    uint8_t* screen; // ���������� ������
    std::unordered_map<char, int> letterToNumber;
    std::ofstream displayFile; // ���� ��� ������
    void drawChar(int x, int y, char c);
    std::unordered_map<char, int> createMap();
};

#endif