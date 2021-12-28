#include <AssetsRegistry.hpp>
#include <SoundHandler.hpp>
#include <GameController.hpp>
#include <GameHandler.hpp>
#include <collision/character_collision.hpp>
#include <levels/EntryLevel.hpp>
#include <screens/GameScreen.hpp>
#include <iostream>

// TODO PIG-12: Initialize the camera on main (avoid global)
extern Vector2D<int> camera_offset;

namespace {
    SDL_Window* create_window()
    {
        auto* window = SDL_CreateWindow(
                WINDOW_TITLE,
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                SDL_WINDOW_SHOWN);
        if (window == nullptr) {
            throw std::runtime_error("SDL Error: Window could not be created");
        }
        return window;
    }

    SDL_Renderer* create_renderer(SDL_Window* window)
    {
        auto* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr) {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        }
        if (renderer == nullptr) {
            throw std::runtime_error("SDL Error: Renderer could not be created");
        }
        return renderer;
    }
}

std::unique_ptr<TitleScreen> GameHandler::create_title_screen(GameHandler* game_handler)
{
    auto on_new_game = [game_handler](){
        game_handler->transition_animation.register_transition_callback([game_handler]() {
            auto game_screen = std::make_unique<GameScreen>(*game_handler);
            game_screen->set_active_level(std::make_unique<EntryLevel>(*game_handler));
            game_handler->screen = std::move(game_screen);
        });
        game_handler->transition_animation.reset();
    };
    auto on_game_exit = [game_handler](){
        game_handler->game_finished = true;
    };
    return std::make_unique<TitleScreen>(on_new_game, on_game_exit);
}

GameHandler::GameHandler()
    : window(create_window())
    , renderer(create_renderer(this->window))
    , screen(GameHandler::create_title_screen(this))
    , game_finished(false)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    assets_registry.load(renderer);
    sound_handler.load();

    // TODO: Move this to the TitleScreen class
    sound_handler.play("title_screen");
}

GameHandler::~GameHandler()
{
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
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
    auto elapsed_time = this->time_handler.get_elapsed_time();

    SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
    SDL_RenderClear(this->renderer);
    this->screen->render(this->renderer, elapsed_time);
    this->transition_animation.run(renderer, elapsed_time);
    SDL_RenderPresent(this->renderer);
}
