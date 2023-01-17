#include <AssetsRegistry.hpp>

void AssetsRegistry::load(SDL_Renderer* renderer)
{
    this->tileset = load_media("assets/sprites/tiles.png", renderer);
    this->lifebar = load_media("assets/sprites/lifebar.png", renderer);
    this->lifebar_heart = load_media("assets/sprites/small_heart18x14.png", renderer);
    this->monogram = load_media("assets/sprites/monogram.png", renderer);
    this->talk_baloon = load_media("assets/sprites/talk_baloon.png", renderer);
    this->forest_background = load_media("assets/sprites/forest_background.png", renderer);
}

AssetsRegistry::~AssetsRegistry()
{
    SDL_DestroyTexture(this->tileset);
    SDL_DestroyTexture(this->lifebar);
    SDL_DestroyTexture(this->lifebar_heart);
    SDL_DestroyTexture(this->monogram);
    SDL_DestroyTexture(this->talk_baloon);
    SDL_DestroyTexture(this->forest_background);
}

AssetsRegistry assets_registry;
