#ifndef PIGSGAME_ISCREEN_HPP
#define PIGSGAME_ISCREEN_HPP

#include <GameController.hpp>
#include <sdl_wrappers.hpp>

class IScreen
{
public:
    virtual void handle_controller(GameController const& keystates) = 0;
    virtual void update(double elapsed_time) = 0;
    virtual void render(SDL_Renderer* renderer) const = 0;
};

#endif //PIGSGAME_ISCREEN_HPP
