#include <drawing.hpp>

Vector2D<int> to_world_position(
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

Vector2D<int> to_camera_position(
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
    SDL_RendererFlip const& flip
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
    SDL_RendererFlip const& flip
) {
    auto srcrect = SDL_Rect{sprite_offset.x, sprite_offset.y, size.x, size.y};
    auto camera_position = to_camera_position(static_camera_position, size, {0, 0});
    auto dstrect = SDL_Rect{camera_position.x, camera_position.y, SCALE_SIZE * size.x, SCALE_SIZE * size.y};
    SDL_RenderCopyEx(renderer, spritesheet, &srcrect, &dstrect, 0.0, nullptr, flip);
}

void gout(
    SDL_Renderer* renderer,
    SDL_Texture* spritesheet,
    Vector2D<int> const& static_camera_position,
    std::string const& message,
    RGBColor const& text_color
)
{
    auto size = Vector2D<int>{6, 9};
    auto srcrect = SDL_Rect{0, 0, size.x, size.y};
    auto dstrect = SDL_Rect{static_camera_position.x, static_camera_position.y, size.x, size.y};
    auto const& charmap = MonogramFont::charmap();

    SDL_SetTextureColorMod(spritesheet, text_color.r, text_color.g, text_color.b);
    for (auto const& c : message) {
        auto const& charmap_pos = charmap.at(c);
        srcrect.x = size.x * charmap_pos.x;
        srcrect.y = size.y * charmap_pos.y;
        SDL_RenderCopy(renderer, spritesheet, &srcrect, &dstrect);
        dstrect.x += size.x;
    }
}
