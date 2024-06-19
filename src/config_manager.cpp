#include "config_manager.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

ConfigManager::ConfigManager(const std::string& baseDir) : baseDir(baseDir), currentDir("") {
    if (!fs::exists(baseDir) || !fs::is_directory(baseDir)) {
        throw std::runtime_error("Base directory does not exist or is not a directory");
    }
}

std::vector<std::string> ConfigManager::getAvailableDirectories() {
    std::vector<std::string> dirs;
    for (const auto& entry : fs::directory_iterator(baseDir)) {
        if (entry.is_directory()) {
            dirs.push_back(entry.path().filename().string());
        }
    }
    return dirs;
}

bool ConfigManager::setConfigDirectory(const std::string& dir) {
    std::string cfgPath = baseDir + "/" + dir + "/yolov3-tiny.cfg";
    std::string weightsPath = baseDir + "/" + dir + "/yolov3-tiny.weights";
    std::string namesPath = baseDir + "/" + dir + "/coco.names";

    if (fs::exists(cfgPath) && fs::exists(weightsPath) && fs::exists(namesPath)) {
        currentDir = dir;
        return true;
    } else {
        return false;
    }
}

std::string ConfigManager::getConfigDirectory() const {
    return baseDir + "/" + currentDir;
}
