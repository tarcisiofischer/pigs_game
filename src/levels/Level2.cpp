#include <GameHandler.hpp>
#include <TransitionAnimation.hpp>
#include <characters/IGameCharacter.hpp>
#include <characters/King.hpp>
#include <characters/builder.hpp>
#include <io.hpp>
#include <levels/EntryLevel.hpp>
#include <levels/Level2.hpp>
#include <screens/GameScreen.hpp>

Level2::Level2(GameHandler& game_handler)
    : map(load_map("maps/level2.map"))
    , characters(build_game_characters(game_handler.get_renderer(), map))
    , game_handler(game_handler)
{
}

GameMap& Level2::get_map()
{
    return this->map;
}

std::vector<IGameCharacter*>& Level2::get_characters()
{
    return this->characters;
}

std::function<void()> Level2::get_collision_callback(int callback_collision_id, IGameCharacter* character)
{
    if (callback_collision_id == 1) {
        auto& game_handler = this->game_handler;
        return [&game_handler]() {
            auto& transition = game_handler.get_transition_animation();
            if (transition.current_state() == TransitionAnimationState::finished) {
                transition.register_transition_callback(
                    [&]() {
                        auto game_screen = dynamic_cast<GameScreen*>(game_handler.get_active_screen());
                        game_screen->set_active_level(std::make_unique<EntryLevel>(game_handler));
                    }
                );
                transition.reset();
            }
        };
    }
    return nullptr;
}
