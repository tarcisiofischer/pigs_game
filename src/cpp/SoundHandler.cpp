#include <SoundHandler.hpp>
#include <logging.hpp>

namespace {
    Mix_Music* load_music(std::string const& filename)
    {
        auto* music = Mix_LoadMUS(filename.c_str());
        if (!music) {
            warn("Unable to load music (filename="s + filename + ": "s + Mix_GetError());
        }
        return music;
    }

    Mix_Chunk* load_sound(std::string const& filename)
    {
        auto* sound = Mix_LoadWAV(filename.c_str());
        if (!sound) {
            warn("Unable to load sound (filename="s + filename + ": "s + Mix_GetError());
        }
        return sound;
    }
}

SoundHandler::SoundHandler()
{
    if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
        warn("Failed to init required OGG support: "s + Mix_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        warn("Sound could not be initialized: "s + Mix_GetError());
    }
}

SoundHandler::~SoundHandler()
{
    for (auto const& [_, music] : this->music_registry) {
        Mix_FreeMusic(music);
    }
    for (auto const& [_, sound] : this->sound_registry) {
        Mix_FreeChunk(sound);
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

void SoundHandler::play_music(std::string const& music_name)
{
    if (!this->music_registry.contains(music_name)) {
        warn("Unknown music named \""s + music_name + "\". Perhaps the sound handler isn't ready?");
        return;
    }

    if (Mix_FadeInMusic(this->music_registry.at(music_name), -1, 2000) == -1) {
        warn("Unable to play music: "s + Mix_GetError());
    }
}

void SoundHandler::play(std::string const& sound_name)
{
    if (!this->sound_registry.contains(sound_name)) {
        warn("Unknown sound named \""s + sound_name + "\". Perhaps the sound handler isn't ready?");
        return;
    }

    if (Mix_PlayChannel(-1, this->sound_registry.at(sound_name), 0) == -1) {
        warn("Unable to play sound: "s + Mix_GetError());
    }
}

void SoundHandler::load()
{
    using namespace std::string_literals;
    for (auto const& filename : {"title_screen"s, "forest"s}) {
        this->music_registry[filename] = load_music("assets/music/"s + filename + ".ogg"s);
    }
    for (auto const& filename : {"hit"s}) {
        this->sound_registry[filename] = load_sound("assets/music/"s + filename + ".ogg"s);
    }
}

SoundHandler sound_handler;
