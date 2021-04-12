#ifndef __DRAWING_HPP
#define __DRAWING_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <Vector2D.hpp>
#include <constants.hpp>
#include <bitmap_font.hpp>

Vector2D<int> to_world_position(
    Vector2D<int> const& camera_position,
    Vector2D<int> const& size,
    Vector2D<int> const& camera_offset
);

Vector2D<int> to_camera_position(
    Vector2D<int> const& world_position,
    Vector2D<int> const& size,
    Vector2D<int> const& camera_offset
);

void draw_sprite(
    SDL_Renderer* renderer,
    SDL_Texture* spritesheet,
    Vector2D<int> const& sprite_offset,
    Vector2D<int> const& world_position,
    Vector2D<int> const& size,
    Vector2D<int> const& camera_offset,
    SDL_RendererFlip const& flip=SDL_FLIP_NONE
);

void draw_static_sprite(
    SDL_Renderer* renderer,
    SDL_Texture* spritesheet,
    Vector2D<int> const& sprite_offset,
    Vector2D<int> const& static_camera_position,
    Vector2D<int> const& size,
    SDL_RendererFlip const& flip=SDL_FLIP_NONE
);

void draw_direct_sprite(
    SDL_Renderer* renderer,
    SDL_Texture* spritesheet,
    Vector2D<int> const& sprite_offset,
    Vector2D<int> const& sdlwindow_position,
    Vector2D<int> const& size
);

void gout(
    SDL_Renderer* renderer,
    SDL_Texture* spritesheet,
    Vector2D<int> const& static_camera_position,
    std::string const& message,
    RGBColor const& text_color
);

template<typename T>
SDL_Rect to_sdl_rect(Region2D<T> const& region)
{
    return SDL_Rect{
        int(region.x), int(region.y),
        int(region.w), int(region.h)};
}

#endif
