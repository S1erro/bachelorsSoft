#include "image_processing.h"
#include "sound.h"
#include <iostream>
#include <fstream>
#include <iomanip> // Для std::setw и std::setfill
#include <linux/input.h> // Для input_event и EV_KEY
#include <unistd.h> // Для read
#include <fcntl.h> // Для O_RDONLY и O_NONBLOCK

ImageProcessor::ImageProcessor() {
    // Открытие камеры
    capture.open(0); // 0 - номер камеры
    if(!capture.isOpened()) {
        std::cerr << "Failed to open camera\n";
        exit(1);
    }
    // Инициализация espeak в новом модуле
    initSound();
}

void ImageProcessor::loadNetwork(const std::string& cfgPath, const std::string& weightsPath, const std::string& namesPath) {
    // Загрузка модели
    net = load_network((char*)cfgPath.c_str(), (char*)weightsPath.c_str(), 0);
    set_batch_network(net, 1);

    // Загрузка меток
    names = get_labels((char*)namesPath.c_str());
}

image ImageProcessor::mat_to_image(cv::Mat m) {
    int h = m.rows;
    int w = m.cols;
    int c = m.channels();
    image im = make_image(w, h, c);
    unsigned char* data = m.data;

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            for (int k = 0; k < c; ++k) {
                im.data[k * w * h + i * w + j] = data[i * w * c + j * c + k] / 255.0;
            }
        }
    }
    return im;
}

void ImageProcessor::recognizeAndDisplay(Display& display, sf::RenderWindow& window, int input_fd) {
    bool exitRecognize = false;
    int photoCount = 0; // Счетчик фотографий
    bool firstRecognition = true; // Флаг для первого распознавания

    cv::Mat frame;
    image im = make_image(0, 0, 0); // Пустое изображение, будет инициализированно позже

    while (!exitRecognize) {
        // Захват кадра
        capture >> frame;
        if (frame.empty()) {
            std::cerr << "Не удалось захватить кадр\n";
            continue;
        }

        // Вывод о распознавании до первого обнаруженного объекта
        if (firstRecognition) {
            display.clear();
            display.drawText(0, 0, "Идет распознавание...");
            display.draw(window);
        }

        if (im.data) {
            // Освобождение предыдущих данных, если они существуют
            free_image(im);
        }
        im = mat_to_image(frame);
        image sized = letterbox_image(im, net->w, net->h);

        // Предсказание
        float* X = sized.data;
        network_predict(net, X);

        // Получение результатов
        int nboxes = 0;
        detection* dets = get_network_boxes(net, im.w, im.h, 0.5, 0.5, 0, 1, &nboxes);

        // Создаем строку распознанных объектов
        std::ostringstream detectedObjectsStream;
        bool objectDetected = false;

        // Обработка результатов
        for (int i = 0; i < nboxes; i++) {
            int best_class = max_index(dets[i].prob, 80);
            float prob = dets[i].prob[best_class];
            if (prob > 0.5) {
                objectDetected = true;
                std::string label = names[best_class];
                std::string text = "Обнаружен объект " + label;
                playSound(text, "ru"); 

                // Добавляем распознанный объект в очередь
                detectedObjects.push_front(label);
                if (detectedObjects.size() > 7) {
                    detectedObjects.pop_back(); // Удаляем старые записи, чтобы хранить последние 7
                }

                // Добавляем объект в строку
                detectedObjectsStream << label << " ";

                // Обновляем экран
                display.clear();
                int y = 0;
                for (const std::string& obj : detectedObjects) {
                    display.drawText(0, y, obj);
                    y += 8; // Переход к следующей строке
                }
                display.draw(window);
            }
        }

        if (objectDetected) {
            firstRecognition = false; // Ставим флаг первого объекта в false
            // Сохраняем кадр и распознанные объекты
            photoCount++;
            std::string photoFilename = "../photos/" + std::to_string(photoCount) + "_photo.jpg";
            std::string textFilename = "../texts/" + std::to_string(photoCount) + "_text.txt";

            cv::imwrite(photoFilename, frame);

            std::ofstream textFile(textFilename);
            textFile << detectedObjectsStream.str();
            textFile.close();
        }

        free_detections(dets, nboxes);
        free_image(sized);

        // Проверка нажатия клавиши для выхода
        struct input_event ev;
        while (read(input_fd, &ev, sizeof(ev)) > 0) {
            if (ev.type == EV_KEY && ev.value == 1 && ev.code == KEY_LEFT) { // Нажатие клавиши влево
                exitRecognize = true;
                break;
            }
        }
    }

    // Освобождаем данныеы
    if (im.data) {
        free_image(im);
    }
}
