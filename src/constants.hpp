#ifndef __CONSTANTS_HPP
#define __CONSTANTS_HPP

#include <array>

auto constexpr WIDTH = 25;
auto constexpr HEIGHT = 18;
auto constexpr SCALE_SIZE = 2;
auto constexpr TILE_SIZE = 32;
auto constexpr SCREEN_WIDTH = 600 * SCALE_SIZE;
auto constexpr SCREEN_HEIGHT = 300 * SCALE_SIZE;

auto constexpr gravity = -0.001;

auto constexpr collision_tiles = std::array<int, 10>{1, 3, 12, 14, 15, 25, 27, 37, 38, 39};

#endif

