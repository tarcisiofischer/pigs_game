#include <characters/Cannon.hpp>
#include <characters/CannonBall.hpp>
#include <characters/Liv.hpp>
#include <characters/Pig.hpp>
#include <items/Key.hpp>
#include <collision/character_collision.hpp>

void pig_liv_collision(Pig* pig_ptr, Liv* liv_ptr)
{
    auto& player = *liv_ptr;
    auto& pig = *pig_ptr;

    auto const& pig_collision_region = pig.get_collision_region_information().collision_region;
    auto const& player_collision_region = player.get_collision_region_information().collision_region;

    if (!player.is_taking_damage && !player.after_taking_damage && !player.is_dying && !player.is_dead && !pig.is_taking_damage && !pig.is_dying && !pig.is_dead) {
        if (check_aabb_collision(player_collision_region, pig_collision_region)) {
            player.start_taking_damage();
        }
    }
}

void cannon_liv_collision(Cannon* cannon_ptr, Liv* liv_ptr)
{
    // Not implemented.
}

void cannonball_liv_collision(CannonBall* cannon_ptr, Liv* liv_ptr)
{
    auto& player = *liv_ptr;
    auto& cannonball = *cannon_ptr;
    if (!player.is_taking_damage && !player.after_taking_damage && !player.is_dying && !player.is_dead) {
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

    if (!pig.is_taking_damage && !pig.is_dying && !pig.is_dead) {
        auto const& cannonball_collision_region = cannonball.get_collision_region_information().collision_region;
        auto const& pig_collision_region = pig.get_collision_region_information().collision_region;

        if (check_aabb_collision(cannonball_collision_region, pig_collision_region)) {
            pig.start_taking_damage();
        }
    }
}

void item_liv_collision(Key* key_ptr, Liv* liv_ptr)
{
    auto& player = *liv_ptr;
    auto& key = *key_ptr;

    auto const& key_collision_region = key.get_collision_region_information().collision_region;
    auto const& king_collision_region = player.get_collision_region_information().collision_region;
    if (check_aabb_collision(key_collision_region, king_collision_region)) {
        key.collect();
    }
}

void compute_characters_collisions(std::vector<std::unique_ptr<IGameCharacter>>& game_characters)
{
    for (int i = 0; i < game_characters.size(); ++i) {
        for (int j = i + 1; j < game_characters.size(); ++j) {
            auto* char_i = game_characters[i].get();
            auto* char_j = game_characters[j].get();

            if (false) {
            }

            else if (dynamic_cast<Pig*>(char_i) && dynamic_cast<Liv*>(char_j)) {
                pig_liv_collision(dynamic_cast<Pig*>(char_i), dynamic_cast<Liv*>(char_j));
            } else if (dynamic_cast<Pig*>(char_j) && dynamic_cast<Liv*>(char_i)) {
                pig_liv_collision(dynamic_cast<Pig*>(char_j), dynamic_cast<Liv*>(char_i));
            }

            else if (dynamic_cast<Cannon*>(char_i) && dynamic_cast<Liv*>(char_j)) {
                cannon_liv_collision(dynamic_cast<Cannon*>(char_i), dynamic_cast<Liv*>(char_j));
            } else if (dynamic_cast<Cannon*>(char_j) && dynamic_cast<Liv*>(char_i)) {
                cannon_liv_collision(dynamic_cast<Cannon*>(char_j), dynamic_cast<Liv*>(char_i));
            }

            else if (dynamic_cast<CannonBall*>(char_i) && dynamic_cast<Liv*>(char_j)) {
                cannonball_liv_collision(dynamic_cast<CannonBall*>(char_i), dynamic_cast<Liv*>(char_j));
            } else if (dynamic_cast<CannonBall*>(char_j) && dynamic_cast<Liv*>(char_i)) {
                cannonball_liv_collision(dynamic_cast<CannonBall*>(char_j), dynamic_cast<Liv*>(char_i));
            }

            else if (dynamic_cast<CannonBall*>(char_i) && dynamic_cast<Pig*>(char_j)) {
                cannonball_pig_collision(dynamic_cast<CannonBall*>(char_i), dynamic_cast<Pig*>(char_j));
            } else if (dynamic_cast<CannonBall*>(char_j) && dynamic_cast<Pig*>(char_i)) {
                cannonball_pig_collision(dynamic_cast<CannonBall*>(char_j), dynamic_cast<Pig*>(char_i));
            }

            else if (dynamic_cast<Key*>(char_i) && dynamic_cast<Liv*>(char_j)) {
                item_liv_collision(dynamic_cast<Key*>(char_i), dynamic_cast<Liv*>(char_j));
            } else if (dynamic_cast<Key*>(char_j) && dynamic_cast<Liv*>(char_i)) {
                item_liv_collision(dynamic_cast<Key*>(char_j), dynamic_cast<Liv*>(char_i));
            }
        }
    }

    game_characters.erase(std::remove_if(game_characters.begin(), game_characters.end(),
                              [](std::unique_ptr<IGameCharacter>& c) {
                                  auto* pig = dynamic_cast<Pig*>(c.get());
                                  if (pig != nullptr) {
                                      return pig->is_dead;
                                  }
                                  return false;
                              }),
        game_characters.end());
}
