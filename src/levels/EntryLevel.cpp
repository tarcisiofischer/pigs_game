#include <GameHandler.hpp>
#include <TransitionAnimation.hpp>
#include <characters/IGameCharacter.hpp>
#include <characters/King.hpp>
#include <characters/builder.hpp>
#include <io.hpp>
#include <levels/EntryLevel.hpp>
#include <levels/Level2.hpp>

EntryLevel::EntryLevel(GameHandler& game_handler)
    : map(load_map("maps/entry_level.map"))
    , characters(build_game_characters(game_handler.get_renderer(), map))
    , game_handler(game_handler)
{
}

GameMap& EntryLevel::get_map()
{
    return this->map;
}

std::vector<IGameCharacter*>& EntryLevel::get_characters()
{
    return this->characters;
}

std::function<void()> EntryLevel::get_collision_callback(int callback_collision_id, IGameCharacter* character)
{
    if (callback_collision_id == 1) {
        auto* player = dynamic_cast<King*>(character);
        if (player) {
            auto& game_handler = this->game_handler;
            return [&game_handler]() {
                auto& transition = game_handler.get_transition_animation();
                if (transition.current_state() == TransitionAnimationState::finished) {
                    transition.register_transition_callback(
                        [&]() { game_handler.set_active_level(std::make_unique<Level2>(game_handler)); });
                    transition.reset();
                }
            };
        }
    }
    return nullptr;
}
