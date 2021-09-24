#include <GameHandler.hpp>

#include <GameController.hpp>
#include <AssetsRegistry.hpp>

#include <constants.hpp>
#include <drawing.hpp>
#include <collision/tilemap_collision.hpp>
#include <collision/character_collision.hpp>

#include <characters/King.hpp>

// TODO PIG-12: Initialize the camera on main (avoid global)
extern Vector2D<int> camera_offset;

GameHandler::GameHandler(SDL_Window* window)
    : active_lvl(nullptr)
    , renderer(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED))
    , enable_debug(false)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    assets_registry.load(renderer);
}

void GameHandler::set_active_level(std::unique_ptr<IGameLevel>&& lvl)
{
    this->active_lvl = std::move(lvl);

    // if (player) {
    //     player->register_on_dead_callback([&transition_animation]() {
    //         transition_animation.reset();
    //     });
    //     player->on_start_taking_damage = [&window_is_shaking, &window_shaker]() {
    //         window_is_shaking = true;
    //         window_shaker.restart();
    //     };
    //     transition_animation.register_transition_callback([&player]() {
    //         player->set_position(100.0, 100.0);
    //         player->is_dead = false;
    //         player->life = 2;
    //     });
    // }
}

SDL_Renderer* GameHandler::get_renderer()
{
    return this->renderer;
}

bool GameHandler::process_inputs()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
            case SDL_QUIT: {
                return false;
            }
            case SDL_KEYDOWN: {
                if (e.key.keysym.sym == SDLK_TAB && !e.key.repeat) {
                    this->enable_debug = !this->enable_debug;
                    break;
                }
            }
        }
    }

    game_controller.update();
    auto keystates = SDL_GetKeyboardState(NULL);
    auto player = this->player();
    if (player) {
        player->handle_controller(keystates);
    }
    return true;
}

void GameHandler::update(double elapsed_time)
{
    this->update_characters(elapsed_time);
    this->compute_collisions();
    this->window_shaker.update(elapsed_time);
}

King* GameHandler::player()
{
    auto& characters = this->active_lvl->get_characters();
    for (auto& c : characters) {
        auto* k = dynamic_cast<King*>(c);
        if (k != nullptr) {
            return k;
        }
    }
    return nullptr;
}

void GameHandler::render(double elapsed_time)
{
    if (this->enable_debug) {
        this->debug_messages.clear();
        // this->debug_messages.push_back("FPS: " + std::to_string(fps));
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    auto const& map = this->active_lvl->get_map();
    auto const& game_characters = this->active_lvl->get_characters();
    auto player = this->player();

    // Draw background
    auto shake = this->window_shaker.get_shake();
    // TODO PIG-11: Only draw what's in the screen
    for (int i = 0; i < map.height; ++i) {
        for (int j = 0; j < map.width; ++j) {
            // Background
            {
                auto tile_id = map.tilemap[i][j];
                auto offset = Vector2D<int>{TILE_SIZE * (tile_id % 12), TILE_SIZE * int(floor(tile_id / 12))};
                auto world_position = Vector2D<int>{TILE_SIZE * j + shake.x, TILE_SIZE * (map.height - i - 1) + shake.y};
                auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
                draw_sprite(renderer, assets_registry.tileset, offset, world_position, size, camera_offset);

                if (this->enable_debug) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 40);
                    for (auto&& collision_tile_id : collision_tiles) {
                        if (tile_id == collision_tile_id) {
                            auto camera_position = to_camera_position(world_position, size, camera_offset);
                            auto dstrect = SDL_Rect{camera_position.x, camera_position.y, SCALE_SIZE * size.x, SCALE_SIZE * size.y};
                            SDL_RenderFillRect(renderer, &dstrect);
                        }
                    }
                }
            }

            // Foreground
            {
                auto tile_id = map.foreground[i][j];
                auto offset = Vector2D<int>{TILE_SIZE * (tile_id % 7), TILE_SIZE * int(floor(tile_id / 7))};
                auto world_position = Vector2D<int>{TILE_SIZE * j + shake.x, TILE_SIZE * (map.height - i - 1) + shake.y};
                auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
                draw_sprite(renderer, assets_registry.foreground_set, offset, world_position, size, camera_offset);
            }
        }
    }
    
    // HUD
    if (player) {
        {
            auto offset = Vector2D<int>{0, 0};
            auto size = Vector2D<int>{66, 34};
            auto static_camera_position = Vector2D<int>{10, SCREEN_HEIGHT / SCALE_SIZE - size.y - 10};
            draw_static_sprite(renderer, assets_registry.lifebar, offset, static_camera_position, size);
        }

        auto offset = Vector2D<int>{0, 0};
        auto size = Vector2D<int>{18, 14};
        for (int i = 0; i < player->life; ++i) {
            auto camera_position = Vector2D<int>{21 + 11 * i, SCREEN_HEIGHT / SCALE_SIZE - size.y - 20};
            draw_static_sprite(renderer, assets_registry.lifebar_heart, offset, camera_position, size);
        }
    }

    for (auto& game_character : game_characters) {
        game_character->run_animation(elapsed_time);
    }
    // TODO PIG-13: Move this to somewhere else; Should only exist when transition is active.
    this->transition_animation.run(renderer, elapsed_time);

    if (this->enable_debug) {
        int mousex = 0;
        int mousey = 0;
        SDL_GetMouseState(&mousex, &mousey);
        this->debug_messages.push_back("Mouse (Camera ): " + std::to_string(mousex) + ", " + std::to_string(mousey));
        auto world_mouse = to_world_position(Vector2D<int>{mousex, mousey}, Vector2D<int>{0, 0}, camera_offset);
        this->debug_messages.push_back("Mouse (World): " + std::to_string(int(world_mouse.x)) + ", " + std::to_string(int(world_mouse.y)));

        auto r = (Uint8)(0);
        auto g = (Uint8)(0);
        auto b = (Uint8)(0);
        auto a = (Uint8)(0);
        SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

        auto debug_area_rect = to_sdl_rect(Region2D<int>{0, 0, SCREEN_WIDTH, 80});

        SDL_SetRenderDrawColor(renderer, 65, 60, 70, 220);
        SDL_RenderFillRect(renderer, &debug_area_rect);
        auto text_position = Vector2D<int>{10, 10};
        for (auto const& message : this->debug_messages) {
            gout(renderer, assets_registry.monogram, text_position, message, RGBColor{100, 240, 100});
            text_position.y += 10;
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 90);
        for (auto& game_character : game_characters) {
            auto const& collision_region = game_character->get_collision_region_information().collision_region;
            auto camera_position = to_camera_position(
                Vector2D<int>{int(collision_region.x), int(collision_region.y)},
                Vector2D<int>{int(collision_region.w), int(collision_region.h)},
                camera_offset
            );
            auto collision_rect = to_sdl_rect(Region2D<int>{
                camera_position.x,
                camera_position.y,
                int(SCALE_SIZE * collision_region.w),
                int(SCALE_SIZE * collision_region.h)
            });
            SDL_RenderFillRect(renderer, &collision_rect);
        }

        SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }

    // Update camera
    {
        auto position = Vector2D<int>{0, 0};
        if (player) {
            position = player->get_position().as_int();
        }

        auto camera_min_x = 0;
        auto user_centered_camera_x = position.x - SCREEN_WIDTH / (2 * SCALE_SIZE);
        auto camera_max_x = std::max(0, map.width * TILE_SIZE - SCREEN_WIDTH / 2);

        auto camera_min_y = 0;
        auto user_centered_camera_y = position.y - SCREEN_HEIGHT / (2 * SCALE_SIZE);
        auto camera_max_y = std::max(0, map.height * TILE_SIZE - SCREEN_HEIGHT / 2);

        camera_offset.x = std::max(camera_min_x, std::min(user_centered_camera_x, camera_max_x));
        camera_offset.y = std::max(camera_min_y, std::min(user_centered_camera_y, camera_max_y));
    }

    SDL_RenderPresent(renderer);
}

void GameHandler::update_characters(double elapsed_time)
{
    auto& game_characters = this->active_lvl->get_characters();

    for (auto* c : game_characters) {
        c->update(elapsed_time);
    }
}

void GameHandler::compute_collisions()
{
    auto& game_characters = this->active_lvl->get_characters();
    auto& map = this->active_lvl->get_map();

    for (auto* c : game_characters) {
        compute_tilemap_collisions(map, c);
    }
    compute_characters_collisions(game_characters);
}

TransitionAnimation& GameHandler::get_transition_animation()
{
    return this->transition_animation;
}
