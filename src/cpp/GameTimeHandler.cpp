#include <GameTimeHandler.hpp>
#include <sdl_wrappers.hpp>

GameTimeHandler::GameTimeHandler()
        : last(0ull)
        , current(SDL_GetPerformanceCounter())
        , fps_countdown(1000.)
        , fps_counter(0)
        , fps(0)
{}

void GameTimeHandler::update()
{
    this->last = this->current;
    this->current = SDL_GetPerformanceCounter();
    this->elapsed_time = (double)(current - last) * 1000.0 / (double)SDL_GetPerformanceFrequency();
    this->fps_countdown -= elapsed_time;
    this->fps_counter += 1;
    if (this->fps_countdown < 0.) {
        this->fps = this->fps_counter;
        this->fps_counter = 0;
        this->fps_countdown = 1000.;
    }
}
