#include <iostream>
#include <sdl_wrappers.hpp>

SDL_Texture* load_media(std::string const& filename, SDL_Renderer* renderer)
{
    auto* surface = IMG_Load(filename.c_str());
    if (surface == nullptr) {
        std::cout << "WARNING: Unable to load image. SDL Error: " << SDL_GetError() << std::endl;
    }
    auto* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        std::cout << "WARNING: Unable to create texture from image. SDL Error: " << SDL_GetError() << std::endl;
        std::cout << "\tImage filename: " << filename << std::endl;
        std::cout << "\tRenderer pointer: " << renderer << std::endl;
    }
    SDL_FreeSurface(surface);
    return texture;
}

SDL_Handler::SDL_Handler()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        throw std::runtime_error("ERROR: SDL could not be initialized! SDL Error: " + std::string(SDL_GetError()));
    }
    if (TTF_Init() != 0) {
        throw std::runtime_error("FAILED to initialize TTF library");
    }
}

SDL_Handler::~SDL_Handler() {
    SDL_Quit();
}
