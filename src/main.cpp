#include <GameHandler.hpp>
#include <GameTimeHandler.hpp>
#include <SDL.h>
#include <Vector2D.hpp>
#include <constants.hpp>
#include <levels/EntryLevel.hpp>
#include <memory>
#include <optional>
#include <sdl_wrappers.hpp>

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

    auto time_handler = GameTimeHandler{};
    auto game_handler = GameHandler(window);
    game_handler.set_active_level(std::make_unique<EntryLevel>(game_handler));

    while (true) {
        time_handler.update();
        auto elapsed_time = time_handler.get_elapsed_time();

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
