#ifndef __SDL_WRAPPERS_HPP
#define __SDL_WRAPPERS_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <string>

SDL_Texture* load_media(std::string const& filename, SDL_Renderer* renderer);
void initialize_sdl();

#endif
