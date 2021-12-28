#ifndef PIGSGAME_SOUNDHANDLER_HPP
#define PIGSGAME_SOUNDHANDLER_HPP

#include <SDL_mixer.h>
#include <unordered_map>
#include <string>

class SoundHandler {
public:
    SoundHandler();
    ~SoundHandler();

    void load();
    void play(std::string const& sound_name);

private:
    std::unordered_map<std::string, Mix_Music*> registry;
};

extern SoundHandler sound_handler;

#endif //PIGSGAME_SOUNDHANDLER_HPP
