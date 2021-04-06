#include <sdl_wrappers.hpp>

SDL_Texture* load_media(std::string const& filename, SDL_Renderer* renderer)
{
    auto* surface = IMG_Load(filename.c_str());
    if (surface == nullptr) {
        std::cout << "WARNING: Unable to load image. SDL Error: " << SDL_GetError() << std::endl;
    }
    auto* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr){
        std::cout << "WARNING: Unable to create texture from image. SDL Error: " << SDL_GetError() << std::endl;
    }
    SDL_FreeSurface(surface);
    return texture;
}
