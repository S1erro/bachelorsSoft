#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <vector>

class ConfigManager {
public:
    ConfigManager(const std::string& baseDir);
    std::vector<std::string> getAvailableDirectories();
    bool setConfigDirectory(const std::string& dir);
    std::string getConfigDirectory() const;

private:
    std::string baseDir;
    std::string currentDir;
};

#endif
