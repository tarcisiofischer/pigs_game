#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <cstring>
#include <algorithm>
#include <iostream>
#include <functional>
#include <optional>
#include <vector>
#include <tuple>
#include <memory>
#include <map>
#include <variant>

#include <characters/IGameCharacter.hpp>
#include <characters/Pig.hpp>
#include <characters/Cannon.hpp>
#include <characters/CannonBall.hpp>
#include <characters/PigWithMatches.hpp>
#include <characters/King.hpp>

#include <GameController.hpp>
#include <SceneScript.hpp>
#include <TransitionAnimation.hpp>
#include <StateTimeout.hpp>
#include <Animation.hpp>
#include <Vector2D.hpp>

#include <collision/aabb.hpp>
#include <collision/CollisionRegion.hpp>
#include <collision/tilemap_collision.hpp>
#include <collision/character_collision.hpp>

#include <constants.hpp>
#include <sdl_wrappers.hpp>
#include <random.hpp>
#include <io.hpp>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

std::vector<std::string> debug_messages;

// TODO PIG-12: Initialize the camera on main (avoid global)
Vector2D<int> camera_offset{0, 0};

std::vector<IGameCharacter*> build_game_characters(SDL_Renderer* renderer, GameMap const& map)
{
    // TODO: Avoid raw pointers
    auto game_characters = std::vector<IGameCharacter*>();
    auto player = ([&map, &renderer]() -> IGameCharacter* {
        for (auto const& info : map.interactables) {
            if (info.id == 0) {
                return new King(renderer, info.position.x, info.position.y);
            }
        }
        return nullptr;
    })();
    if (player) {
        game_characters.push_back(player);
    }

    for (auto const& info : map.interactables) {
        if (info.id == 1) {
            auto pig = new Pig(renderer, info.position.x, info.position.y);
            game_characters.push_back(pig);
        }
    }

    return game_characters;
}

auto constexpr ACTIVE_MAP = "intro.map";

void prepare_script(std::vector<IGameCharacter*> game_characters)
{
    auto pig1 = dynamic_cast<Pig*>(game_characters[0]);
    auto pig2 = dynamic_cast<Pig*>(game_characters[1]);
    auto pig3 = dynamic_cast<Pig*>(game_characters[2]);

    pig1->set_script({{
        {0, std::make_unique<WalkTo>(pig1, 130)},
        {5, std::make_unique<FaceTo>(pig1, -1)},
        {10, std::make_unique<Talk>(pig1, "Hey man")},
    }});
    pig2->set_script({{
        {0, std::make_unique<WalkTo>(pig2, 100)},
        {5, std::make_unique<FaceTo>(pig2, +1)},
        {6, std::make_unique<WaitScriptEvent>(pig1, 2)},
        {10, std::make_unique<Talk>(pig2, "Sup'")},
    }});
    pig3->set_script({{
        {0, std::make_unique<WalkTo>(pig3, 80)},
        {5, std::make_unique<FaceTo>(pig3, +1)},
        {6, std::make_unique<WaitScriptEvent>(pig1, 2)},
        {10, std::make_unique<Talk>(pig3, "Sup")},
    }});
}

int main(int argc, char* args[])
{
    SDL_Window* window = nullptr;
    bool quit = false;
    SDL_Event e;

    initialize_sdl();
    auto* default_font = TTF_OpenFont("./FreeMono.ttf", 12);
    if (default_font == nullptr) {
        throw std::runtime_error("Unable to create default TTF font.");
    }

    window = SDL_CreateWindow(
        "Pigs game",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (window == nullptr) {
        throw std::runtime_error("SDL Error: Window could not be created");
    }

    auto* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool show_debug = false;
    
    auto load_spritesheet = [&renderer](std::string const& filename) {
        return load_media("assets/sprites/" + filename, renderer);
    };
    auto tileset = load_spritesheet("tileset.png");
    auto foreground_set = load_spritesheet("foreground_set.png");
    auto lifebar = load_spritesheet("lifebar.png");
    auto lifebar_heart = load_spritesheet("small_heart18x14.png");
    auto door = load_spritesheet("door46x56.png");
    auto monogram = load_spritesheet("monogram.png");

    auto window_is_shaking = false;
    auto window_shaker = StateTimeout(300., [&window_is_shaking](){ window_is_shaking = false; });

    auto map = load_map(ACTIVE_MAP);
    auto const& tilemap = map.tilemap;
    auto const& foreground = map.foreground;
    auto const& interactibles = map.interactables;

    auto game_characters = build_game_characters(renderer, map);
    auto player = dynamic_cast<King*>(game_characters[0]);

    prepare_script(game_characters);

    auto last = (unsigned long long)(0);
    auto current = SDL_GetPerformanceCounter();
    auto fps_countdown = 1000.;
    auto fps_counter = 0;
    auto fps = 0;
    
    auto transition_animation = TransitionAnimation();
    if (player) {
        player->register_on_dead_callback([&transition_animation]() {
            transition_animation.reset();
        });
        player->on_start_taking_damage = [&window_is_shaking, &window_shaker]() {
            window_is_shaking = true;
            window_shaker.restart();
        };
        transition_animation.register_transition_callback([&player]() {
            player->set_position(100.0, 100.0);
            player->is_dead = false;
            player->life = 2;
        });
    }

    while (!quit) {
        last = current;
        current = SDL_GetPerformanceCounter();
        double elapsedTime = (double)((current - last)*1000.0 / (double)SDL_GetPerformanceFrequency() );
        fps_countdown -= elapsedTime;
        fps_counter += 1;
        if (fps_countdown < 0.) {
            fps = fps_counter;
            fps_counter = 0;
            fps_countdown = 1000.;
        }

        debug_messages.clear();
        debug_messages.push_back("FPS: " + std::to_string(fps));

        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT: {
                    quit = true;
                    break;
                }
                case SDL_KEYDOWN: {
                    if (e.key.keysym.sym == SDLK_TAB && !e.key.repeat) {
                        show_debug = !show_debug;
                        break;
                    }
                }
            }
        }
        game_controller.update();
        auto keystates = SDL_GetKeyboardState(NULL);
        if (player) {
            player->handle_controller(keystates);
        }

        for (auto* c : game_characters) {
            c->update(elapsedTime);
        }

        for (auto* c : game_characters) {
            compute_tilemap_collisions(map, c);
        }
        compute_characters_collisions(game_characters);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw background
        int shake_x = window_is_shaking ? random_int(-3, 3) : 0;
        int shake_y = window_is_shaking ? random_int(-1, 1) : 0;
        window_shaker.update(elapsedTime);
        // TODO PIG-11: Only draw what's in the screen
        for (int i = 0; i < map.height; ++i) {
            for (int j = 0; j < map.width; ++j) {
                // Background
                {
                    auto tile_id = tilemap[i][j];
                    auto offset = Vector2D<int>{TILE_SIZE * (tile_id % 12), TILE_SIZE * int(floor(tile_id / 12))};
                    auto world_position = Vector2D<int>{TILE_SIZE * j + shake_x, TILE_SIZE * (map.height - i - 1) + shake_y};
                    auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
                    draw_sprite(renderer, tileset, offset, world_position, size, camera_offset);

                    if (show_debug) {
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
                    auto tile_id = foreground[i][j];
                    auto offset = Vector2D<int>{TILE_SIZE * (tile_id % 7), TILE_SIZE * int(floor(tile_id / 7))};
                    auto world_position = Vector2D<int>{TILE_SIZE * j + shake_x, TILE_SIZE * (map.height - i - 1) + shake_y};
                    auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
                    draw_sprite(renderer, foreground_set, offset, world_position, size, camera_offset);
                }
            }
        }
        
        // HUD
        if (player) {
            {
                auto offset = Vector2D<int>{0, 0};
                auto size = Vector2D<int>{66, 34};
                auto static_camera_position = Vector2D<int>{10, SCREEN_HEIGHT / SCALE_SIZE - size.y - 10};
                draw_static_sprite(renderer, lifebar, offset, static_camera_position, size);
            }

            auto offset = Vector2D<int>{0, 0};
            auto size = Vector2D<int>{18, 14};
            for (int i = 0; i < player->life; ++i) {
                auto camera_position = Vector2D<int>{21 + 11 * i, SCREEN_HEIGHT / SCALE_SIZE - size.y - 20};
                draw_static_sprite(renderer, lifebar_heart, offset, camera_position, size);
            }
        }

        for (auto& game_character : game_characters) {
            game_character->run_animation(elapsedTime);
        }
        // TODO PIG-13: Move this to somewhere else; Should only exist when transition is active.
        transition_animation.run(renderer, elapsedTime);

        int mousex = 0;
        int mousey = 0;
        SDL_GetMouseState(&mousex, &mousey);
        debug_messages.push_back("Mouse (Camera ): " + std::to_string(mousex) + ", " + std::to_string(mousey));
        auto world_mouse = to_world_position(Vector2D<int>{mousex, mousey}, Vector2D<int>{0, 0}, camera_offset);
        debug_messages.push_back("Mouse (World): " + std::to_string(int(world_mouse.x)) + ", " + std::to_string(int(world_mouse.y)));
        if (show_debug) {
            auto r = (Uint8)(0);
            auto g = (Uint8)(0);
            auto b = (Uint8)(0);
            auto a = (Uint8)(0);
            SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

            auto debug_area_rect = to_sdl_rect(Region2D<int>{0, 0, SCREEN_WIDTH, 80});

            SDL_SetRenderDrawColor(renderer, 65, 60, 70, 220);
            SDL_RenderFillRect(renderer, &debug_area_rect);
            auto text_position = Vector2D<int>{10, 10};
            for (auto const& message : debug_messages) {
                gout(renderer, monogram, text_position, message, RGBColor{100, 240, 100});
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

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

