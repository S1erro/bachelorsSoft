#include <SFML/Graphics.hpp>
#include <iostream>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <thread>
#include <chrono>
#include "display.h"
#include "image_processing.h"
#include "sound.h"
#include "config_manager.h"

#define WIDTH 128
#define HEIGHT 64

enum MenuOption {
    Start,
    YoloSettings,
    Volume,
    MenuOptionCount
};

void drawMenu(Display& display, MenuOption selected) {
    display.clear();
    std::string options[MenuOptionCount] = {"1. Распознавание", "2. Конфигурация Yolo", "3. Громкость"};
    for (int i = 0; i < MenuOptionCount; ++i) {
        if (i == selected) {
            display.drawText(0, i * 8, ">");
        }
        display.drawText(6, i * 8, options[i]);
    }
}

void drawVolumeMenu(Display& display, int selectedVolume) {
    display.clear();
    std::string levels[5] = {"1. Беззвучный", "2. Уровень 1", "3. Уровень 2", "4. Уровень 3", "5. Уровень 4"};
    for (int i = 0; i < 5; ++i) {
        if (i == selectedVolume) {
            display.drawText(0, i * 8, ">");
        }
        display.drawText(6, i * 8, levels[i]);
    }
}

void drawYoloSettings(Display& display, const std::vector<std::string>& dirs, int selectedDir) {
    display.clear();
    for (size_t i = 0; i < dirs.size(); ++i) {
        if (i == selectedDir) {
            display.drawText(0, i * 8, ">");
        }
        display.drawText(6, i * 8, dirs[i]);
    }
}

std::string getExecutablePath() {
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string fullPath(buffer);
        return fullPath.substr(0, fullPath.find_last_of("/"));
    }
    return "";
}

int main() {
    std::string baseDir = getExecutablePath() + "/../config_files";
    std::cout << "Using base directory: " << baseDir << std::endl;

    Display display(WIDTH, HEIGHT, "disp.txt");  // Передаем имя файла

    ConfigManager configManager(baseDir);

    std::vector<std::string> availableDirs;
    try {
        availableDirs = configManager.getAvailableDirectories();
        if (availableDirs.empty()) {
            std::cerr << "No available directories found in " << baseDir << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error initializing ConfigManager: " << e.what() << std::endl;
        return 1;
    }

    ImageProcessor imgProcessor;

    MenuOption currentOption = Start;
    int currentVolume = 0;
    bool inMenu = true;
    bool inVolumeMenu = false;
    bool inYoloSettingsMenu = false;
    int selectedDir = 0;

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Emulator");

    // Открытие устройства ввода
    int fd = open("/dev/input/event2", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Failed to open /dev/input/event2: " << strerror(errno) << std::endl;
        return 1;
    }

    initSound(); // Инициализация звука
while (window.isOpen()) {
        struct input_event ev;
        while (read(fd, &ev, sizeof(ev)) > 0) {
            if (ev.type == EV_KEY && ev.value == 1) { // Только нажатия клавиш
                if (inMenu) {
                    if (ev.code == KEY_UP) {
                        if (currentOption > 0) {
                            currentOption = static_cast<MenuOption>(currentOption - 1);
                        }
                    } else if (ev.code == KEY_DOWN) {
                        if (currentOption < MenuOptionCount - 1) {
                            currentOption = static_cast<MenuOption>(currentOption + 1);
                        }
                    } else if (ev.code == KEY_RIGHT) {
                        if (currentOption == Volume) {
                            inVolumeMenu = true;
                            inMenu = false;
                        } else if (currentOption == Start) {
                            inMenu = false;
                            display.clear();
                            imgProcessor.recognizeAndDisplay(display, window, fd); // Передаем объект дисплея и окно
                            inMenu = true;
                        } else if (currentOption == YoloSettings) {
                            inYoloSettingsMenu = true;
                            inMenu = false;
                        }
                    }
                } else if (inVolumeMenu) {
                    if (ev.code == KEY_UP) {
                        if (currentVolume > 0) {
                            currentVolume--;
                        }
                    } else if (ev.code == KEY_DOWN) {
                        if (currentVolume < 4) {
                            currentVolume++;
                        }
                    } else if (ev.code == KEY_RIGHT) {
                        playBeep(currentVolume);
                        inVolumeMenu = false;
                        inMenu = true;
                    } else if (ev.code == KEY_LEFT) {
                        inVolumeMenu = false;
                        inMenu = true;
                    }
                } else if (inYoloSettingsMenu) {
                    if (ev.code == KEY_UP) {
                        if (selectedDir > 0) {
                            selectedDir--;
                        }
                    } else if (ev.code == KEY_DOWN) {
                        if (selectedDir < availableDirs.size() - 1) {
                            selectedDir++;
                        }
                    } else if (ev.code == KEY_RIGHT) {
                        if (configManager.setConfigDirectory(availableDirs[selectedDir])) {
                            std::string configDir = configManager.getConfigDirectory();
                            imgProcessor.loadNetwork(configDir + "/yolov3-tiny.cfg",
                                                     configDir + "/yolov3-tiny.weights",
                                                     configDir + "/coco.names");
                            display.drawText(0, 0, "Config " + availableDirs[selectedDir] + " acc");
                            playSound("Конфигурация " + availableDirs[selectedDir] + " применена", "ru");
                        } else {
                            display.clear();
                            display.drawText(0, 0, "Недостаточно файлов");
                            playSound("Недостаточно файлов в конфигурации", "ru");
                        }
                        display.draw(window);
                        std::this_thread::sleep_for(std::chrono::seconds(2)); // Даем пользователю время увидеть сообщение
                        inYoloSettingsMenu = false;
                        inMenu = true;
                    } else if (ev.code == KEY_LEFT) {
                        inYoloSettingsMenu = false;
                        inMenu = true;
                    }
                } else if (ev.code == KEY_LEFT) {
                    inMenu = true;
                }
            }
        }
// Рисование и обновление окна
        if (inMenu) {
            drawMenu(display, currentOption);
        } else if (inVolumeMenu) {
            drawVolumeMenu(display, currentVolume);
        } else if (inYoloSettingsMenu) {
            drawYoloSettings(display, availableDirs, selectedDir);
        }

        display.draw(window);

        // Проверка и обработка событий окна
        sf::Event windowEvent;
        while (window.pollEvent(windowEvent)) {
            if (windowEvent.type == sf::Event::Closed) {
                window.close();
            }
        }
    }

    display.saveToFile();  // Обновляем вызов метода
    close(fd); // Закрываем устройство ввода
    cleanupSound(); // Освобождаем ресурсы звука
    return 0;
}