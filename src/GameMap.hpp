#ifndef __GAMEMAP_HPP
#define __GAMEMAP_HPP

#include <Vector2D.hpp>
#include <vector>
#include <array>

using Tilemap = std::vector<std::vector<int>>;
auto constexpr collision_tiles = std::array<int, 10>{1, 3, 12, 14, 15, 25, 27, 37, 38, 39};
auto constexpr foreground_collision_tiles = std::array<int, 9>{2, 3, 4, 5, 9, 10, 11, 12, 21};

struct InteractableInfo {
    Vector2D<int> position;
    int id;
    int flip;
};

struct GameMap {
    int width;
    int height;
    Tilemap tilemap;
    Tilemap foreground;
    std::vector<InteractableInfo> interactables;

    GameMap(int width, int height);
};

#endif
