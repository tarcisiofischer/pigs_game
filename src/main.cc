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

#include <AssetsRegistry.hpp>
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

static std::map<std::string, std::string> tr = {
    {
        "Hah! It worked.",
        "Ha! Deu certo."
    },
    {
        "Boss'll like to know we managed to stole Otto's treasure",
        "O chefe vai gostar de saber que conseguimos roubar o cofre do Otto!"
    },
    {
        "Y-yeah, but it'll t-t-take some t-time to take all this g-gold f-f-from here...",
        "M-mas vai levar um t-tempo pra gente tirar t-todo esse ouro d-da-daqui..."
    },
    {
        "Well... If you talk less and start working, it'll be faster",
        "Bom, se voce falar menos e comecar a trabalhar, vai ser mais rapido"
    },
    {
        "What if Otto wake up before we finish, boss?",
        "Mas e se o Otto acordar antes de a gente terminar, chefe?"
    },
    {
        "Yesterday was his birthday, man.",
        "Ontem foi o aniversario do cara."
    },
    {
        "He must be sleepin' after the party.",
        "Ele deve estar acabado na cama..."
    },
    {
        "We'll have plenty of time to steal everything.",
        "Vamos ter tempo de sobra pra carregar tudo."
    },
    {
        "Y-yeah, b-but my back hu-hurts, you know?",
        "Hmm... M-mas minhas costas estao m-me-meio doloridas, s-sabe?"
    },
    {
        "Now that you mentioned... My feet hurts a bit",
        "Po, eu to com um calo nas patas"
    },
    {
        "Did y-you see I've g-g-got this weird stain in my nose?",
        "T-tu v-viu que eu p-p-peguei uma p-pereba aqui no meu f-fu-fucinho?"
    },
    {
        "Wow! creepy, man. You should see a doctor.",
        "Po, que tenso, cara."
    },
    {
        "My mother-in-law had a similar thing on she's nose and...",
        "Minha sogra pegou uma pereba assim esses dias, melhor vc ir no medico"
    },
    {
        "Can you two stop the smalltalk...",
        "Voces dois podem parar de papinho..."
    },
    {
        "AND START WORKING??",
        "E COMECAR A TRABALHAR??"
    }
};

void prepare_script(std::vector<IGameCharacter*> game_characters)
{
    auto pig1 = dynamic_cast<Pig*>(game_characters[0]);
    auto pig2 = dynamic_cast<Pig*>(game_characters[1]);
    auto pig3 = dynamic_cast<Pig*>(game_characters[2]);

    auto pig1_color = RGBColor{0, 100, 0};
    auto pig2_color = RGBColor{250, 50, 50};
    auto pig3_color = RGBColor{20, 30, 250};

    #define _(T) std::make_unique<T>
    pig1->set_script({{
        {1, _(WalkTo)(130)},
        {5, _(FaceTo)(-1)},
        {10, _(Talk)(tr["Hah! It worked."], pig1_color)},
        {15, _(Talk)(tr["Boss'll like to know we managed to stole Otto's treasure"], pig1_color)},
        // {20, _(Talk)("heheheh.")},
        {25, _(WaitScriptEvent)(pig2, 15)},
        {30, _(Talk)(tr["Well... If you talk less and start working, it'll be faster"], pig1_color)},
        {35, _(WaitScriptEvent)(pig3, 15)},
        {40, _(Talk)(tr["Yesterday was his birthday, man."], pig1_color)},
        {45, _(Talk)(tr["He must be sleepin' after the party."], pig1_color)},
        {50, _(Talk)(tr["We'll have plenty of time to steal everything."], pig1_color)},
        {51, _(WaitScriptEvent)(pig2, 45)},
        {52, _(SetAngry)(true)},
        {55, _(WaitScriptEvent)(pig3, 40)},
        {60, _(Talk)(tr["Can you two stop the smalltalk..."], pig1_color)},
        {65, _(Talk)(tr["AND START WORKING??"], pig1_color)},
    }});
    pig2->set_script({{
        {0, _(WaitTime)(1000.0)},
        {0, _(WalkTo)(100)},
        {5, _(FaceTo)(+1)},
        {10, _(WaitScriptEvent)(pig1, 15)},
        {15, _(Talk)(tr["Y-yeah, but it'll t-t-take some t-time to take all this g-gold f-f-from here..."], pig2_color)},
        {20, _(WaitScriptEvent)(pig3, 10)},
        {25, _(FaceTo)(-1)},
        {26, _(WaitScriptEvent)(pig1, 35)},
        {27, _(FaceTo)(+1)},
        {30, _(WaitScriptEvent)(pig1, 50)},
        {35, _(Talk)(tr["Y-yeah, b-but my back hu-hurts, you know?"], pig2_color)},
        {25, _(FaceTo)(-1)},
        {40, _(WaitScriptEvent)(pig3, 25)},
        {45, _(Talk)(tr["Did y-you see I've g-g-got this weird stain in my nose?"], pig2_color)},
        {50, _(WaitScriptEvent)(pig1, 55)},
        {55, _(FaceTo)(+1)},
        {60, _(WaitScriptEvent)(pig1, 60)},
        {65, _(SetFear)(true)},
        {70, _(WaitScriptEvent)(pig1, 65)},
    }});
    pig3->set_script({{
        {0, _(WaitTime)(500.0)},
        {1, _(WalkTo)(80)},
        {5, _(FaceTo)(+1)},
        {10, _(WaitScriptEvent)(pig1, 30)},
        {15, _(Talk)(tr["What if Otto wake up before we finish, boss?"], pig3_color)},
        {20, _(WaitScriptEvent)(pig2, 35)},
        {25, _(Talk)(tr["Now that you mentioned... My feet hurts a bit"], pig3_color)},
        {30, _(WaitScriptEvent)(pig2, 45)},
        {35, _(Talk)(tr["Wow! creepy, man. You should see a doctor."], pig3_color)},
        {40, _(Talk)(tr["My mother-in-law had a similar thing on she's nose and..."], pig3_color)},
        {45, _(WaitScriptEvent)(pig1, 60)},
        {50, _(SetFear)(true)},
        {55, _(WaitScriptEvent)(pig1, 65)},
    }});
    #undef _
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
    assets_registry.load(renderer);

    bool show_debug = false;
    
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
                    draw_sprite(renderer, assets_registry.tileset, offset, world_position, size, camera_offset);

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

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

