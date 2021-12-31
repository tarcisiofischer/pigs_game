#ifndef __CONSTANTS_HPP
#define __CONSTANTS_HPP

#ifndef FORCE_SCALE_SIZE
#define FORCE_SCALE_SIZE 2
#endif

auto constexpr WINDOW_TITLE = "Pigs Castle";
auto constexpr SCALE_SIZE = FORCE_SCALE_SIZE;
auto constexpr TILE_SIZE = 16;
auto constexpr SCREEN_WIDTH = 600 * SCALE_SIZE;
auto constexpr SCREEN_HEIGHT = 300 * SCALE_SIZE;

auto constexpr gravity = -0.001;

#endif
