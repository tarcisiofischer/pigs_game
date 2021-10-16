#include <GameHandler.hpp>
#include <SDL.h>
#include <Vector2D.hpp>
#include <constants.hpp>
#include <levels/EntryLevel.hpp>
#include <memory>
#include <optional>
#include <sdl_wrappers.hpp>

// TODO PIG-12: Initialize the camera on main (avoid global)
Vector2D<int> camera_offset { 0, 0 };

int main(int argc, char* args[])
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
    game_handler.set_active_level(std::make_unique<EntryLevel>(game_handler));

    auto last = (unsigned long long)(0);
    auto current = SDL_GetPerformanceCounter();
    auto fps_countdown = 1000.;
    auto fps_counter = 0;
    auto fps = 0;

    while (true) {
        last = current;
        current = SDL_GetPerformanceCounter();
        double elapsed_time = (double)((current - last) * 1000.0 / (double)SDL_GetPerformanceFrequency());
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
