#ifndef __CHARACTERS_COLLISION
#define __CHARACTERS_COLLISION

#include <characters/Pig.hpp>
#include <characters/Cannon.hpp>
#include <characters/King.hpp>

#include <collision/aabb.hpp>

void pig_king_collision(Pig* pig_ptr, King* king)
{
    auto& player = *king;
    auto& pig = *pig_ptr;

    auto const& pig_collision_region = pig.get_collision_region_information().collision_region;
    auto const& player_collision_region = player.get_collision_region_information().collision_region;

    if (
        player.is_attacking &&
        !player.is_taking_damage &&
        !pig.is_taking_damage &&
        !pig.is_dying &&
        !pig.is_dead
    ) {
        auto player_attack_region = player.attack_region();

        if (check_aabb_collision(player_attack_region, pig_collision_region)) {
            pig.start_taking_damage();
        }
    }

    if (
        !player.is_taking_damage &&
        !player.after_taking_damage &&
        !player.is_dying &&
        !player.is_dead &&
        !pig.is_taking_damage &&
        !pig.is_dying &&
        !pig.is_dead
    ) {
        if (check_aabb_collision(player_collision_region, pig_collision_region)) {
            player.start_taking_damage();
        }
    }
}

void cannon_king_collision(Cannon* cannon_ptr, King* king)
{
    auto& player = *king;
    auto& cannon = *cannon_ptr;

    if (
        player.is_attacking &&
        !player.is_taking_damage
    ) {
        auto const& cannon_collision_region = cannon.get_collision_region_information().collision_region;
        auto player_attack_region = player.attack_region();
        
        if (check_aabb_collision(player_attack_region, cannon_collision_region)) {
            cannon.trigger_attack();
        }
    }
}

void cannonball_king_collision(CannonBall* cannon_ptr, King* king)
{
    auto& player = *king;
    auto& cannonball = *cannon_ptr;
    if (
        !player.is_taking_damage &&
        !player.after_taking_damage &&
        !player.is_dying &&
        !player.is_dead
    ) {
        auto cannonball_collision_region_info = cannonball.get_collision_region_information();
        auto cannonball_collision_region = cannonball_collision_region_info.collision_region;

        auto player_collision_region_info = player.get_collision_region_information();
        auto player_collision_region = player_collision_region_info.collision_region;

        if (check_aabb_collision(cannonball_collision_region, player_collision_region)) {
            player.start_taking_damage();
        }
    }
}

void cannonball_pig_collision(CannonBall* cannon_ptr, Pig* pig_ptr)
{
    auto& pig = *pig_ptr;
    auto& cannonball = *cannon_ptr;

    if (
        !pig.is_taking_damage &&
        !pig.is_dying &&
        !pig.is_dead
    ) {
        auto const& cannonball_collision_region = cannonball.get_collision_region_information().collision_region;
        auto const& pig_collision_region = pig.get_collision_region_information().collision_region;

        if (check_aabb_collision(cannonball_collision_region, pig_collision_region)) {
            pig.start_taking_damage();
        }
    }
}

void compute_characters_collisions(std::vector<IGameCharacter*>& game_characters)
{
    for (int i = 0; i < game_characters.size(); ++i) {
        for (int j = i + 1; j < game_characters.size(); ++j) {
            if (false) {}

            else if (dynamic_cast<Pig*>(game_characters[i]) && dynamic_cast<King*>(game_characters[j])) { pig_king_collision(dynamic_cast<Pig*>(game_characters[i]), dynamic_cast<King*>(game_characters[j])); }
            else if (dynamic_cast<Pig*>(game_characters[j]) && dynamic_cast<King*>(game_characters[i])) { pig_king_collision(dynamic_cast<Pig*>(game_characters[j]), dynamic_cast<King*>(game_characters[i])); }

            else if (dynamic_cast<Cannon*>(game_characters[i]) && dynamic_cast<King*>(game_characters[j])) { cannon_king_collision(dynamic_cast<Cannon*>(game_characters[i]), dynamic_cast<King*>(game_characters[j])); }
            else if (dynamic_cast<Cannon*>(game_characters[j]) && dynamic_cast<King*>(game_characters[i])) { cannon_king_collision(dynamic_cast<Cannon*>(game_characters[j]), dynamic_cast<King*>(game_characters[i])); }

            else if (dynamic_cast<CannonBall*>(game_characters[i]) && dynamic_cast<King*>(game_characters[j])) { cannonball_king_collision(dynamic_cast<CannonBall*>(game_characters[i]), dynamic_cast<King*>(game_characters[j])); }
            else if (dynamic_cast<CannonBall*>(game_characters[j]) && dynamic_cast<King*>(game_characters[i])) { cannonball_king_collision(dynamic_cast<CannonBall*>(game_characters[j]), dynamic_cast<King*>(game_characters[i])); }

            else if (dynamic_cast<CannonBall*>(game_characters[i]) && dynamic_cast<Pig*>(game_characters[j])) { cannonball_pig_collision(dynamic_cast<CannonBall*>(game_characters[i]), dynamic_cast<Pig*>(game_characters[j])); }
            else if (dynamic_cast<CannonBall*>(game_characters[j]) && dynamic_cast<Pig*>(game_characters[i])) { cannonball_pig_collision(dynamic_cast<CannonBall*>(game_characters[j]), dynamic_cast<Pig*>(game_characters[i])); }
        }
    }

    game_characters.erase(
        std::remove_if(game_characters.begin(), game_characters.end(), [](IGameCharacter* c) {
            auto* pig = dynamic_cast<Pig*>(c);
            if (pig != nullptr) {
                return pig->is_dead;
            }
            return false;
        }),
        game_characters.end()
    );
}

#endif
