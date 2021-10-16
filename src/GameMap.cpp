#include <GameMap.hpp>

GameMap::GameMap(int width, int height)
    : width(width)
    , height(height)
    , tilemap { std::vector<std::vector<int>>(height, std::vector<int>(width)) }
    , foreground { std::vector<std::vector<int>>(height, std::vector<int>(width)) }
    , interactables { 0 }
{
}
