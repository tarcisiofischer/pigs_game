#ifndef __DRAWING_HPP
#define __DRAWING_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <Vector2D.hpp>
#include <constants.hpp>

inline Vector2D<int> to_world_position(
    Vector2D<int> const& camera_position,
    Vector2D<int> const& size,
    Vector2D<int> const& camera_offset
)
{
    return {
        camera_position.x / SCALE_SIZE + camera_offset.x,
        ((camera_position.y - SCREEN_HEIGHT) / SCALE_SIZE) * -1 - size.y + camera_offset.y
    };
}

inline Vector2D<int> to_camera_position(
    Vector2D<int> const& world_position,
    Vector2D<int> const& size,
    Vector2D<int> const& camera_offset
)
{
    return {
        SCALE_SIZE * (world_position.x - camera_offset.x),
        SCALE_SIZE * ((world_position.y + size.y - camera_offset.y) / -1) + SCREEN_HEIGHT
    };
}

void draw_sprite(
    SDL_Renderer* renderer,
    SDL_Texture* spritesheet,
    Vector2D<int> const& sprite_offset,
    Vector2D<int> const& world_position,
    Vector2D<int> const& size,
    Vector2D<int> const& camera_offset,
    SDL_RendererFlip const& flip=SDL_FLIP_NONE
) {
    auto srcrect = SDL_Rect{sprite_offset.x, sprite_offset.y, size.x, size.y};
    auto camera_position = to_camera_position(world_position, size, camera_offset);
    auto dstrect = SDL_Rect{camera_position.x, camera_position.y, SCALE_SIZE * size.x, SCALE_SIZE * size.y};
    SDL_RenderCopyEx(renderer, spritesheet, &srcrect, &dstrect, 0.0, nullptr, flip);
}

void draw_static_sprite(
    SDL_Renderer* renderer,
    SDL_Texture* spritesheet,
    Vector2D<int> const& sprite_offset,
    Vector2D<int> const& static_camera_position,
    Vector2D<int> const& size,
    SDL_RendererFlip const& flip=SDL_FLIP_NONE
) {
    auto srcrect = SDL_Rect{sprite_offset.x, sprite_offset.y, size.x, size.y};
    auto camera_position = to_camera_position(static_camera_position, size, {0, 0});
    auto dstrect = SDL_Rect{camera_position.x, camera_position.y, SCALE_SIZE * size.x, SCALE_SIZE * size.y};
    SDL_RenderCopyEx(renderer, spritesheet, &srcrect, &dstrect, 0.0, nullptr, flip);
}


template<typename T>
SDL_Rect to_sdl_rect(Region2D<T> const& region)
{
    return SDL_Rect{
        int(region.x), int(region.y),
        int(region.w), int(region.h)};
}

#endif
