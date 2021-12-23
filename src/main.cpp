#include <GameHandler.hpp>
#include <SDL.h>
#include <Vector2D.hpp>
#include <constants.hpp>
#include <levels/EntryLevel.hpp>
#include <memory>
#include <optional>
#include <sdl_wrappers.hpp>
#include <iostream>

// TODO PIG-12: Initialize the camera on main (avoid global)
Vector2D<int> camera_offset { 0, 0 };

int main()
{
    initialize_sdl();

    auto* window = SDL_CreateWindow(
        "Pigs game",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        throw std::runtime_error("SDL Error: Window could not be created");
    }

    auto game_handler = GameHandler(window);

    while (true) {
        game_handler.process_inputs();
        game_handler.update();
        if (game_handler.is_game_finished()) {
            break;
        }
        game_handler.render();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
