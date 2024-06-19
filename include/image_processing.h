#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <opencv2/opencv.hpp>
#include <deque>
#include <string>
#include <SFML/Graphics.hpp>
#include "display.h"
#include "darknet.h"

class ImageProcessor {
public:
    ImageProcessor();
    void recognizeAndDisplay(Display& display, sf::RenderWindow& window, int input_fd);
    void loadNetwork(const std::string& cfgPath, const std::string& weightsPath, const std::string& namesPath);

private:
    network* net;
    char** names;
    cv::VideoCapture capture;
    std::deque<std::string> detectedObjects;
    image mat_to_image(cv::Mat m);
};

#endif
