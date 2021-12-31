#include <drawing.hpp>

Vector2D<int> to_world_position(Vector2D<int> const& camera_position, Vector2D<int> const& size,
    Vector2D<int> const& camera_offset)
{
    return { camera_position.x / SCALE_SIZE + camera_offset.x,
        ((camera_position.y - SCREEN_HEIGHT) / SCALE_SIZE) * -1 - size.y + camera_offset.y };
}

Vector2D<int> to_camera_position(Vector2D<int> const& world_position, Vector2D<int> const& size,
    Vector2D<int> const& camera_offset)
{
    return { SCALE_SIZE * (world_position.x - camera_offset.x),
        SCALE_SIZE * ((world_position.y + size.y - camera_offset.y) / -1) + SCREEN_HEIGHT };
}

void draw_sprite(SDL_Renderer* renderer, SDL_Texture* spritesheet, Vector2D<int> const& sprite_offset,
    Vector2D<int> const& world_position, Vector2D<int> const& size, Vector2D<int> const& camera_offset,
    SDL_RendererFlip const& flip)
{
    auto srcrect = SDL_Rect { sprite_offset.x, sprite_offset.y, size.x, size.y };
    auto camera_position = to_camera_position(world_position, size, camera_offset);
    auto dstrect = SDL_Rect { camera_position.x, camera_position.y, SCALE_SIZE * size.x, SCALE_SIZE * size.y };
    SDL_RenderCopyEx(renderer, spritesheet, &srcrect, &dstrect, 0.0, nullptr, flip);
}

// Rething about this. Perhaps solve in PIG-12
void draw_static_sprite(SDL_Renderer* renderer, SDL_Texture* spritesheet, Vector2D<int> const& sprite_offset,
    Vector2D<int> const& static_camera_position, Vector2D<int> const& size,
    SDL_RendererFlip const& flip)
{
    auto srcrect = SDL_Rect { sprite_offset.x, sprite_offset.y, size.x, size.y };
    auto camera_position = to_camera_position(static_camera_position, size, { 0, 0 });
    auto dstrect = SDL_Rect { camera_position.x, camera_position.y, SCALE_SIZE * size.x, SCALE_SIZE * size.y };
    SDL_RenderCopyEx(renderer, spritesheet, &srcrect, &dstrect, 0.0, nullptr, flip);
}

void draw_direct_sprite(SDL_Renderer* renderer, SDL_Texture* spritesheet, Vector2D<int> const& sprite_offset,
    Vector2D<int> const& sdlwindow_position, Vector2D<int> const& size)
{
    auto srcrect = SDL_Rect { sprite_offset.x, sprite_offset.y, size.x, size.y };
    auto dstrect = SDL_Rect { sdlwindow_position.x, sdlwindow_position.y, SCALE_SIZE * size.x, SCALE_SIZE * size.y };
    SDL_RenderCopy(renderer, spritesheet, &srcrect, &dstrect);
}

void draw_filled_region(SDL_Renderer* renderer, Region2D<int> const& region, RGBColor const& fill_color)
{
    SDL_SetRenderDrawColor(renderer, fill_color.r, fill_color.g, fill_color.b, 255);
    auto rect = to_sdl_rect(region);
    SDL_RenderFillRect(renderer, &rect);
}

void draw_line(SDL_Renderer* renderer, Vector2D<int> const& start_position, Vector2D<int> const& end_position,
    RGBColor const& fill_color)
{
    SDL_SetRenderDrawColor(renderer, fill_color.r, fill_color.g, fill_color.b, 255);
    SDL_RenderDrawLine(renderer, start_position.x, start_position.y, end_position.x, end_position.y);
}

int gstr_width(std::string const& text)
{
    return text.size() * 6 * SCALE_SIZE;
}

Region2D<int> gout(SDL_Renderer* renderer, SDL_Texture* spritesheet, Vector2D<int> const& static_camera_position,
    std::string const& message, RGBColor const& text_color, bool scale)
{
    auto scale_size = scale ? SCALE_SIZE : 1;
    auto gout_region = Region2D<int> { static_camera_position.x, static_camera_position.y, 0, 0 };

    auto size = Vector2D<int> { 6, 9 };
    auto srcrect = SDL_Rect { 0, 0, size.x, size.y };
    auto dstrect = SDL_Rect { static_camera_position.x, static_camera_position.y, size.x * scale_size, size.y * scale_size };
    auto const& charmap = MonogramFont::charmap();

    SDL_SetTextureColorMod(spritesheet, text_color.r, text_color.g, text_color.b);
    for (auto const& c : message) {
        auto const& charmap_pos = [&]() {
            try {
                return charmap.at(c);
            } catch (std::out_of_range) {
                return charmap.at('?');
            }
        }();
        srcrect.x = size.x * charmap_pos.x;
        srcrect.y = size.y * charmap_pos.y;
        SDL_RenderCopy(renderer, spritesheet, &srcrect, &dstrect);
        dstrect.x += size.x * scale_size;
        gout_region.w += size.x * scale_size;
    }
    gout_region.h += size.y;

    return gout_region;
}
