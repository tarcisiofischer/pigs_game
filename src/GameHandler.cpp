#include <AssetsRegistry.hpp>
#include <GameController.hpp>
#include <GameHandler.hpp>
#include <collision/character_collision.hpp>
#include <levels/EntryLevel.hpp>
#include <screens/GameScreen.hpp>

// TODO PIG-12: Initialize the camera on main (avoid global)
extern Vector2D<int> camera_offset;

std::unique_ptr<TitleScreen> GameHandler::create_title_screen(GameHandler* game_handler)
{
    auto on_new_game = [game_handler](){
        auto game_screen = std::make_unique<GameScreen>(*game_handler);
        game_screen->set_active_level(std::make_unique<EntryLevel>(*game_handler));
        game_handler->screen = std::move(game_screen);
    };
    auto on_game_exit = [game_handler](){
        game_handler->game_finished = true;
    };
    return std::make_unique<TitleScreen>(on_new_game, on_game_exit);
}

GameHandler::GameHandler(SDL_Window* window)
    : renderer(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED))
    , screen(GameHandler::create_title_screen(this))
    , game_finished(false)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    assets_registry.load(renderer);
}

void GameHandler::process_inputs()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            this->game_finished = true;
        }
    }

    game_controller.update();
    this->screen->handle_controller(game_controller);
}

void GameHandler::update()
{
    this->time_handler.update();
    this->screen->update(this->time_handler.get_elapsed_time());
}

void GameHandler::render()
{
    SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
    SDL_RenderClear(this->renderer);
    this->screen->render(this->renderer, this->time_handler.get_elapsed_time());
    SDL_RenderPresent(this->renderer);
}
