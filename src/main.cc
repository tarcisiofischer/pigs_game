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

#include <levels/PreludeLevel.hpp>

#include <GameHandler.hpp>
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
#include <memory>

// TODO PIG-12: Initialize the camera on main (avoid global)
Vector2D<int> camera_offset{0, 0};

int main(int argc, char* args[])
{
    initialize_sdl();

    auto* window = SDL_CreateWindow(
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

    auto game_handler = GameHandler(window);
    game_handler.set_active_level(std::make_unique<PreludeLevel>(game_handler));

    auto last = (unsigned long long)(0);
    auto current = SDL_GetPerformanceCounter();
    auto fps_countdown = 1000.;
    auto fps_counter = 0;
    auto fps = 0;
    
    while (true) {
        last = current;
        current = SDL_GetPerformanceCounter();
        double elapsed_time = (double)((current - last)*1000.0 / (double)SDL_GetPerformanceFrequency() );
        fps_countdown -= elapsed_time;
        fps_counter += 1;
        if (fps_countdown < 0.) {
            fps = fps_counter;
            fps_counter = 0;
            fps_countdown = 1000.;
        }

        if (!game_handler.process_inputs()) {
            break;
        }
        game_handler.update(elapsed_time);
        game_handler.render(elapsed_time);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

