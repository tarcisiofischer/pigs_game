#ifndef PIGSGAME_GAMESCREEN_HPP
#define PIGSGAME_GAMESCREEN_HPP

#include <screens/IScreen.hpp>
#include <levels/IGameLevel.hpp>
#include <memory>
#include <characters/King.hpp>

class GameHandler;

class GameScreen : public IScreen {
public:
    GameScreen(GameHandler& game_handler);
    void handle_controller(GameController const& controller) override;
    void update(double elapsed_time) override;
    void render(SDL_Renderer* renderer, double elapsed_time) override;
    King* player();
    void set_active_level(std::unique_ptr<IGameLevel>&& lvl);

private:
    void update_characters(double elapsed_time);
    void compute_collisions();

private:
    GameHandler& game_handler;
    std::unique_ptr<IGameLevel> active_lvl;
    bool enable_debug;
    std::vector<std::string> debug_messages;
};

#endif //PIGSGAME_GAMESCREEN_HPP
