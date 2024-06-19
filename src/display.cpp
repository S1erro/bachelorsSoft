#include "display.h"
#include "SystemRus5x7.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cstring>

#define charLength 6
#define numOfLines 8
#define rusCharShift 481
#define nonrusCharShift -154
#define lineWidth 128

// ����������� ������ Display
Display::Display(int width, int height, const std::string& filename) : width(width), height(height) {
    screen = new uint8_t[width * (height / 8)](); // ���������� ������ �� 1024 ���������

    displayFile.open(filename, std::ios::out | std::ios::binary);
    if (!displayFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }

    clear();
    letterToNumber = createMap();
}

// ������� �������� ����� ��������
std::unordered_map<char, int> Display::createMap() {
    std::string letters = "����������������������������������������������������������������";
    std::unordered_map<char, int> letterToNumber;

    for (int i = 0; i < letters.size(); ++i) {
        letterToNumber[letters[i]] = i + 1;
    }

    return letterToNumber;
}

// ������� ������� ������
void Display::clear() {
    memset(screen, 0, width * (height / numOfLines));
}

// ������� ��������� �������
void Display::drawChar(int x, int y, char c) {
    const uint8_t* font;
    if (c >= 0x0 && c < 0x80) {
        font = SystemRus5x7 + c - nonrusCharShift;
    } else if (letterToNumber.find(c) != letterToNumber.end()) {
        font = SystemRus5x7 + rusCharShift + (letterToNumber.at(c)) * 5;
    } else {
        // ���������� �������, ������� ����������� � �����
        return;
    }

    // ���������������� ����
    for (int i = 0; i < 5; ++i) {
        if (x + i < width) {
            int pos = (y / numOfLines) * width + x + i;
            if (pos < width * (height / numOfLines)) {
                screen[pos] = font[i];
            }
        }
    }
}

// ������� ��������� ������
void Display::drawText(int x, int y, const std::string& text) {
    for (char c: text) {
        drawChar(x, y, c);
        x += charLength; // ������ ������� + ������
        if (x >= width) break; // ���������, ���� ����� ������� �� ������� ������
    }
}

// ������� ��������� �� ����� (��������, ����� SFML)
void Display::draw(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    sf::RectangleShape pixel(sf::Vector2f(1, 1));
    pixel.setFillColor(sf::Color::White);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (screen[(y / numOfLines) * width + x] & (1 << (y % 8))) {
                pixel.setPosition(x, y);
                window.draw(pixel);
            }
        }
    }
    window.display();
}

// ������� ���������� ������ � ����
void Display::saveToFile() {
    FILE* display;
    display = fopen("disp.txt", "w");
    if (!display) {
        std::cerr << "Failed to open file for writing" << std::endl;
        return;
    }

    for (size_t i = 0; i < 8; ++i) {
        for (int j = 0; j < numOfLines; ++j) {
            for (size_t k = 0; k < lineWidth; ++k) {
                char bit = (screen[i * lineWidth + k] & (1 << j)) ? '#' : ' ';
                fprintf(display, "%c", bit);
            }
            fprintf(display, "\n");
        }
    }

    fclose(display);
}