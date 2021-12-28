#include <GameHandler.hpp>
#include <SDL.h>
#include <Vector2D.hpp>
#include <constants.hpp>
#include <optional>
#include <sdl_wrappers.hpp>

// TODO PIG-12: Initialize the camera on main (avoid global)
Vector2D<int> camera_offset { 0, 0 };

int main()
{
    SDL_Handler _();
    auto game_handler = GameHandler();

    while (true) {
        Uint32 start_time = SDL_GetTicks();

        game_handler.process_inputs();
        game_handler.update();
        if (game_handler.is_game_finished()) {
            break;
        }
        game_handler.render();

        if((1000 / 70.) > (SDL_GetTicks() - start_time)) {
            SDL_Delay((1000 / 70.) - (SDL_GetTicks() - start_time));
        }
    }

    return 0;
}
