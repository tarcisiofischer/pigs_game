#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <Vector2D.hpp>
#include <sdl_wrappers.hpp>
#include <constants.hpp>
#include <drawing.hpp>
#include <collision/aabb.hpp>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>

// TODO PIG-12: Initialize the camera on main (avoid global)
Vector2D<int> camera_offset{0, 0};

struct GameMap {
    int width;
    int height;
    std::vector<std::vector<int>> tilemap;
    std::vector<std::vector<int>> foreground;
};

void save_map(GameMap const& map, std::string const& filename)
{
    std::ofstream mapfile(filename, std::ios::binary | std::ios::out);
    if (!mapfile.is_open()) {
        throw std::runtime_error("Could not open file to write");
    }
    auto bin_write = [&mapfile](int const& data) {
        mapfile.write(reinterpret_cast<const char*>(&data), sizeof(int));
    };

    bin_write(map.width);
    bin_write(map.height);
    for (int i = 0; i < map.height; ++i) {
        for (int j = 0; j < map.width; ++j) {
            bin_write(map.tilemap[i][j]);
        }
    }
    for (int i = 0; i < map.height; ++i) {
        for (int j = 0; j < map.width; ++j) {
            bin_write(map.foreground[i][j]);
        }
    }
    mapfile.close();
}

GameMap load_map(std::string const& filename)
{
    std::ifstream mapfile(filename, std::ios::binary | std::ios::in);
    if (!mapfile.is_open()) {
        throw std::runtime_error("Could not open file to read");
    }

    auto bin_read_nextint = [&mapfile](int &target) {
        mapfile.read(reinterpret_cast<char*>(&target), sizeof(int));
    };

    auto map = GameMap{0, 0};

    bin_read_nextint(map.width);
    bin_read_nextint(map.height);

    map.tilemap = std::vector<std::vector<int>>(map.height, std::vector<int>(map.width));
    for (int i = 0; i < map.height; ++i) {
        for (int j = 0; j < map.width; ++j) {
            bin_read_nextint(map.tilemap[i][j]);
        }
    }

    map.foreground = std::vector<std::vector<int>>(map.height, std::vector<int>(map.width));
    for (int i = 0; i < map.height; ++i) {
        for (int j = 0; j < map.width; ++j) {
            bin_read_nextint(map.foreground[i][j]);
        }
    }
    mapfile.close();

    return map;
}

void draw_background_tileset(
    SDL_Renderer* renderer,
    SDL_Texture* tileset,
    bool is_clicked,
    Vector2D<int> const& mouse,
    // Out:
    int& selected_tile
)
{
    auto constexpr offset = Vector2D<int>{15, 30};
    auto constexpr padding = Vector2D<int>{2, 2};
    auto constexpr N_TILES = 12 * 8;
    auto tileset_x = 0;
    auto tileset_y = 0;
    auto panel_x = 0;
    auto panel_y = 0;
    for (int tile_id = 0; tile_id < N_TILES; ++tile_id) {
        auto tile_position = Vector2D<int>{
            panel_x * TILE_SIZE + offset.x + panel_x * padding.x,
            panel_y * TILE_SIZE + offset.y + panel_y * padding.y
        };
        draw_direct_sprite(
            renderer,
            tileset,
            {tileset_x * TILE_SIZE, tileset_y * TILE_SIZE},
            tile_position,
            {TILE_SIZE, TILE_SIZE}
        );

        tileset_x += 1;
        if (tileset_x % 12 == 0) {
            tileset_x = 0;
            tileset_y += 1;
        }

        panel_x += 1;
        if (panel_x % 5 == 0) {
            panel_x = 0;
            panel_y += 1;
        }

        // Draw selected tile border (if selected)
        auto tile_region = Region2D<int>{tile_position.x, tile_position.y, TILE_SIZE, TILE_SIZE};
        auto mouse_is_over = check_aabb_collision(
            tile_region.as<double>(),
            Region2D<int>{mouse.x, mouse.y, 0, 0}.as<double>()
        );
        auto tile_is_selected = selected_tile == tile_id;
        if (mouse_is_over || tile_is_selected) {
            SDL_SetRenderDrawColor(renderer, 250, 255, 255, 255);
            auto sdl_rect = to_sdl_rect(tile_region);
            SDL_RenderDrawRect(renderer, &sdl_rect);

            if (is_clicked) {
                selected_tile = tile_id;
            }
        }
    }
}

void draw_foreground_tileset(
    SDL_Renderer* renderer,
    SDL_Texture* foreground_tileset,
    bool is_clicked,
    Vector2D<int> const& mouse,
    // Out:
    int& selected_tile
)
{
    auto constexpr offset = Vector2D<int>{15, 30};
    auto constexpr padding = Vector2D<int>{2, 2};
    auto constexpr N_TILES = 7 * 4;
    auto tileset_x = 0;
    auto tileset_y = 0;
    auto panel_x = 0;
    auto panel_y = 0;
    for (int tile_id = 0; tile_id < N_TILES; ++tile_id) {
        auto tile_position = Vector2D<int>{
            panel_x * TILE_SIZE + offset.x + panel_x * padding.x,
            panel_y * TILE_SIZE + offset.y + panel_y * padding.y
        };
        draw_direct_sprite(
            renderer,
            foreground_tileset,
            {tileset_x * TILE_SIZE, tileset_y * TILE_SIZE},
            tile_position,
            {TILE_SIZE, TILE_SIZE}
        );

        tileset_x += 1;
        if (tileset_x % 7 == 0) {
            tileset_x = 0;
            tileset_y += 1;
        }

        panel_x += 1;
        if (panel_x % 4 == 0) {
            panel_x = 0;
            panel_y += 1;
        }

        // Draw selected tile border (if selected)
        auto tile_region = Region2D<int>{tile_position.x, tile_position.y, TILE_SIZE, TILE_SIZE};
        auto mouse_is_over = check_aabb_collision(
            tile_region.as<double>(),
            Region2D<int>{mouse.x, mouse.y, 0, 0}.as<double>()
        );
        auto tile_is_selected = selected_tile == tile_id;
        if (mouse_is_over || tile_is_selected) {
            SDL_SetRenderDrawColor(renderer, 250, 255, 255, 255);
            auto sdl_rect = to_sdl_rect(tile_region);
            SDL_RenderDrawRect(renderer, &sdl_rect);

            if (is_clicked) {
                selected_tile = tile_id;
            }
        }
    }
}

struct Options {
    std::string filename;

    bool new_file;
    int width;
    int height;

    bool open_file;
};

// Options handle_args(int argc, char* argv[])
// {

//     return {
//         filenane,
//         new_file,
//         width,
//         height,
//         open_file
//     }
// }

int main(int argc, char* argv[])
{
    // auto options =  handle_args(argc, argv);

    auto filename = "output.map";
    auto map = load_map(filename);

    initialize_sdl();

    auto window = SDL_CreateWindow(
        "Pigs game - Map editor",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1024,
        768,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (window == nullptr) {
        throw std::runtime_error("SDL Error: Window could not be created");
    }

    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    auto load_spritesheet = [&renderer](std::string const& filename) {
        return load_media("assets/sprites/" + filename, renderer);
    };
    auto tileset = load_spritesheet("tileset.png");
    auto foreground_set = load_spritesheet("foreground_set.png");

    camera_offset.x = -int(map.width * TILE_SIZE / 2 - 200);
    camera_offset.y = int(map.height * TILE_SIZE / 2);

    auto mouse = Vector2D<int>{0, 0};

    auto quit = false;
    SDL_Event e;

    auto constexpr BACKGROUND_SECTION = 1;
    auto constexpr FOREGROUND_SECTION = 2;

    auto is_clicked = false;
    auto is_just_clicked = false;
    int selected_tile = -1;
    int selected_section = BACKGROUND_SECTION;

    while (!quit) {
        is_just_clicked = false;
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT: {
                quit = true;
                break;
                }

                case SDL_MOUSEMOTION: {
                    if (e.motion.state & SDL_BUTTON_RMASK) {
                        camera_offset.x -= e.motion.xrel;
                        camera_offset.y += e.motion.yrel;
                    }
                    mouse.x = e.motion.x;
                    mouse.y = e.motion.y;
                    break;
                }

                case SDL_MOUSEBUTTONDOWN: {
                    is_just_clicked = true;
                    is_clicked = true;
                    break;
                }

                case SDL_MOUSEBUTTONUP: {
                    is_clicked = false;
                    is_just_clicked = false;
                    break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 63, 56, 81, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < map.height; ++i) {
            for (int j = 0; j < map.width; ++j) {
                auto world_position = Vector2D<int>{TILE_SIZE * j, TILE_SIZE * (map.height - i - 1)};
                auto camera_position = to_camera_position(world_position, {TILE_SIZE, TILE_SIZE}, camera_offset);
                auto tile_region = Region2D<int>{camera_position.x, camera_position.y, TILE_SIZE, TILE_SIZE};
                auto mouse_is_over = check_aabb_collision(
                    tile_region.as<double>(),
                    Region2D<int>{mouse.x, mouse.y, 0, 0}.as<double>()
                );

                // Background
                {
                    auto tile_id = map.tilemap[i][j];
                    auto offset = Vector2D<int>{TILE_SIZE * (tile_id % 12), TILE_SIZE * int(floor(tile_id / 12))};
                    auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
                    draw_sprite(renderer, tileset, offset, world_position, size, camera_offset);
                }

                // Foreground
                {
                    auto tile_id = map.foreground[i][j];
                    auto offset = Vector2D<int>{TILE_SIZE * (tile_id % 7), TILE_SIZE * int(floor(tile_id / 7))};
                    auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
                    draw_sprite(renderer, foreground_set, offset, world_position, size, camera_offset);
                }

                // Check if selected
                {
                    if (mouse_is_over) {
                        SDL_SetRenderDrawColor(renderer, 250, 255, 255, 255);
                        auto sdl_rect = to_sdl_rect(tile_region);
                        SDL_RenderDrawRect(renderer, &sdl_rect);

                        if (is_clicked && selected_tile != -1) {
                            if (selected_section == BACKGROUND_SECTION) {
                                map.tilemap[i][j] = selected_tile;
                            } else if (selected_section == FOREGROUND_SECTION) {
                                map.foreground[i][j] = selected_tile;
                            }
                        }
                    }
                }
            }
        }

        // Selected tile (mouse icon)
        {
            auto world_mouse = to_world_position(mouse, {0, 0}, camera_offset);
            world_mouse.x -= TILE_SIZE / 2;
            world_mouse.y -= TILE_SIZE / 2;
            auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
            if (selected_section == BACKGROUND_SECTION) {
                auto offset = Vector2D<int>{TILE_SIZE * (selected_tile % 12), TILE_SIZE * int(floor(selected_tile / 12))};
                draw_sprite(renderer, tileset, offset, world_mouse, size, camera_offset);
            } else if (selected_section == FOREGROUND_SECTION) {
                auto offset = Vector2D<int>{TILE_SIZE * (selected_tile % 7), TILE_SIZE * int(floor(selected_tile / 7))};
                draw_sprite(renderer, foreground_set, offset, world_mouse, size, camera_offset);
            }
        }

        // Drawable region border
        {
            SDL_SetRenderDrawColor(renderer, 250, 200, 150, 255);
            auto map_size = Vector2D<int>{TILE_SIZE * map.width, TILE_SIZE * map.height};
            auto map_position = to_camera_position({0, 0}, map_size, camera_offset);
            auto map_rect = SDL_Rect{map_position.x, map_position.y, map_size.x, map_size.y};
            SDL_RenderDrawRect(renderer, &map_rect);
        }

        // Left panel
        {
            int window_w = 0;
            int window_h = 0;
            SDL_GetWindowSize(window, &window_w, &window_h);
            SDL_SetRenderDrawColor(renderer, 45, 35, 60, 255);
            auto fixed_rect = SDL_Rect{0, 0, 200, window_h};
            SDL_RenderFillRect(renderer, &fixed_rect);

            // Save button
            {
                auto save_btn_region = Region2D<int>{20, 5, 20, 20};

                SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);
                auto fixed_rect = to_sdl_rect(save_btn_region);
                SDL_RenderFillRect(renderer, &fixed_rect);

                auto mouse_is_over = check_aabb_collision(
                    save_btn_region.as<double>(),
                    Region2D<int>{mouse.x, mouse.y, 0, 0}.as<double>()
                );
                if (mouse_is_over) {
                    SDL_SetRenderDrawColor(renderer, 0, 250, 0, 255);
                    SDL_RenderFillRect(renderer, &fixed_rect);

                    if (is_just_clicked) {
                        save_map(map, filename);
                        std::cout << "Map saved in " << filename << std::endl;
                    }
                }
            }

            // "Change tileset" button
            {
                auto change_tile_btn_region = Region2D<int>{45, 5, 20, 20};

                SDL_SetRenderDrawColor(renderer, 0, 0, 150, 255);
                auto fixed_rect = to_sdl_rect(change_tile_btn_region);
                SDL_RenderFillRect(renderer, &fixed_rect);

                auto mouse_is_over = check_aabb_collision(
                    change_tile_btn_region.as<double>(),
                    Region2D<int>{mouse.x, mouse.y, 0, 0}.as<double>()
                );
                if (mouse_is_over) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 250, 255);
                    SDL_RenderFillRect(renderer, &fixed_rect);

                    if (is_just_clicked) {
                        if (selected_section == BACKGROUND_SECTION) {
                            selected_section = FOREGROUND_SECTION;
                        } else if (selected_section == FOREGROUND_SECTION) {
                            selected_section = BACKGROUND_SECTION;
                        }
                        selected_tile = -1;
                    }
                }
            }

            // TILES selection
            {
                if (selected_section == BACKGROUND_SECTION) {
                    draw_background_tileset(
                        renderer,
                        tileset,
                        is_clicked,
                        mouse,
                        selected_tile
                    );
                } else if (selected_section == FOREGROUND_SECTION) {
                    draw_foreground_tileset(
                        renderer,
                        foreground_set,
                        is_clicked,
                        mouse,
                        selected_tile
                    );
                }
            }

        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}