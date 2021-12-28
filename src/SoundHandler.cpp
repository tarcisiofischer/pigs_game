#include <SoundHandler.hpp>
#include <iostream>
#include <string>

namespace {
    Mix_Music* load_sound(std::string const& filename)
    {
        auto* music = Mix_LoadMUS(filename.c_str());
        if (!music) {
            std::cout << "WARNING: Unable to load sound. SDL Error: " << Mix_GetError() << " (Filename: " << filename << ")" << std::endl;
        }
        return music;
    }
}

SoundHandler::SoundHandler()
{
    if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
        std::cout << "Warning: Failed to init required OGG support! Details: " << Mix_GetError() << std::endl;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "Warning: Sound could not be initialized. Details: " << Mix_GetError() << std::endl;
    }
}

SoundHandler::~SoundHandler()
{
    for (auto const& [_, music] : this->registry) {
        Mix_FreeMusic(music);
    }

    // From the SDL_mixer docs: Since each call to Mix_Init may set different flags,
    //     there is no way, currently, to request how many times each one was initted.
    //     In other words, the only way to quit for sure is to do a loop like so:
    // https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_frame.html
    //
    while(Mix_Init(0)) {
        Mix_Quit();
    }
}

void SoundHandler::play(std::string const& sound_name)
{
    if (!this->registry.contains(sound_name)) {
        std::cout << "WARNING: Unknown " << sound_name << ". Perhaps the sound handler isn't ready?" << std::endl;
        return;
    }

    if (Mix_FadeInMusic(this->registry.at(sound_name), -1, 2000) == -1) {
        std::cout << "WARNING: Unable to play sound. SDL Error: " << Mix_GetError() << std::endl;
    }
}

void SoundHandler::load()
{
    using namespace std::string_literals;
    for (auto const& filename : {"title_screen"s, "forest"s}) {
        this->registry[filename] = load_sound("assets/music/"s + filename + ".ogg"s);
    }
}

SoundHandler sound_handler;
