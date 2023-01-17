#include <GameHandler.hpp>
#include <sdl_wrappers.hpp>

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
