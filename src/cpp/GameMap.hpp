#ifndef __GAMEMAP_HPP
#define __GAMEMAP_HPP

#include <Vector2D.hpp>
#include <array>
#include <vector>

using Tilemap = std::vector<std::vector<int>>;

struct InteractableInfo {
    Vector2D<int> position;
    int id;
    int flip;
};

struct GameMap {
    int width;
    int height;
    Tilemap tilemap0;
    Tilemap tilemap1;
    Tilemap tilemap2;
    std::vector<InteractableInfo> interactables;

    GameMap(int width, int height);
};

#endif
