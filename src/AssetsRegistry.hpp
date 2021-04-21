#ifndef __ASSETS_REGISTRY_H
#define __ASSETS_REGISTRY_H

#include <SDL2/SDL.h>
#include <sdl_wrappers.hpp>

struct AssetsRegistry {
    void load(SDL_Renderer *renderer);
    ~AssetsRegistry();

    SDL_Texture *tileset;
    SDL_Texture *foreground_set;
    SDL_Texture *lifebar;
    SDL_Texture *lifebar_heart;
    SDL_Texture *door;
    SDL_Texture *monogram;
    SDL_Texture *talk_baloon;
};

extern AssetsRegistry assets_registry;

#endif
