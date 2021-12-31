#include <GameHandler.hpp>
#include <Vector2D.hpp>
#include <sdl_wrappers.hpp>

// TODO PIG-12: Initialize the camera on main (avoid global)
Vector2D<int> camera_offset { 0, 0 };

int main()
{
    SDL_Handler _;
    auto game_handler = GameHandler();
    while (!game_handler.is_game_finished()) {
        game_handler.process_inputs();
        game_handler.update();
        game_handler.render();
        game_handler.delay();
    }
    return 0;
}
