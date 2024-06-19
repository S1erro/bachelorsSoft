#include "sound.h"
#include <espeak/speak_lib.h>
#include <alsa/asoundlib.h>
#include <iostream>

static snd_mixer_t* handle = nullptr;
static snd_mixer_elem_t* elem = nullptr;

void initSound() {
    // Инициализация espeak
    espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0);

    // ALSA
    const char* card = "default";
    const char* selem_name = "Master";

    if (snd_mixer_open(&handle, 0) < 0) {
        std::cerr << "Failed to open mixer\n";
        return;
    }
    if (snd_mixer_attach(handle, card) < 0) {
        std::cerr << "Failed to attach mixer\n";
        snd_mixer_close(handle);
        handle = nullptr;
        return;
    }
    if (snd_mixer_selem_register(handle, NULL, NULL) < 0) {
        std::cerr << "Failed to register mixer\n";
        snd_mixer_close(handle);
        handle = nullptr;
        return;
    }
    if (snd_mixer_load(handle) < 0) {
        std::cerr << "Failed to load mixer\n";
        snd_mixer_close(handle);
        handle = nullptr;
        return;
    }

    snd_mixer_selem_id_t* sid;
    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    elem = snd_mixer_find_selem(handle, sid);
    snd_mixer_selem_id_free(sid);

    if (!elem) {
        std::cerr << "Failed to find mixer element\n";
        snd_mixer_close(handle);
        handle = nullptr;
    }
}

void playSound(const std::string& text, const std::string& lang) {
    espeak_SetVoiceByName(lang.c_str());
    espeak_Synth(text.c_str(), text.size() + 1, 0, POS_CHARACTER, 0, espeakCHARS_AUTO, NULL, NULL);
}

void setVolume(int level) {
    if (!handle || !elem) {
        std::cerr << "Mixer not initialized\n";
        return;
    }

    long min, max;
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    long volume = min + level * (max - min) / 4; // делим на 4, так как 5 уровней (0-4)
    snd_mixer_selem_set_playback_volume_all(elem, volume);
}

void playBeep(int level) {
    setVolume(level);
    
    system("aplay /usr/share/sounds/alsa/Front_Center.wav");
}

void cleanupSound() {
    if (handle) {
        snd_mixer_close(handle);
        handle = nullptr;
        elem = nullptr;
    }
}
