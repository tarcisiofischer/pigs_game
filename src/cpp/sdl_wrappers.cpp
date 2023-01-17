#include <sdl_wrappers.hpp>
#include <logging.hpp>

SDL_Texture* load_media(std::string const& filename, SDL_Renderer* renderer)
{
    auto* surface = IMG_Load(filename.c_str());
    if (surface == nullptr) {
        warn("Unable to load image. SDL Error: "s + SDL_GetError());
    }
    auto* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        warn("Unable to create texture from image. SDL Error: "s + SDL_GetError());
    }
    SDL_FreeSurface(surface);
    return texture;
}

SDL_Handler::SDL_Handler()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        err("SDL could not be initialized! SDL Error: "s + SDL_GetError());
    }
    if (TTF_Init() != 0) {
        err("Failed to initialize TTF library");
    }
}

SDL_Handler::~SDL_Handler() {
    TTF_Quit();
    SDL_Quit();
}
