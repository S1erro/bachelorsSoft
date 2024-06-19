#ifndef SOUND_H
#define SOUND_H

#include <string>

void initSound();
void playSound(const std::string& text, const std::string& lang = "en");
void setVolume(int level);
void playBeep(int level);
void cleanupSound();

#endif
