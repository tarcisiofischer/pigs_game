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

auto constexpr BACKGROUND_SECTION = 1;
auto constexpr FOREGROUND_SECTION = 2;

struct GameMap {
    int width;
    int height;
    std::vector<std::vector<int>> tilemap;
    std::vector<std::vector<int>> foreground;

    GameMap(int width, int height)
        : width(width)
        , height(height)
        , tilemap{std::vector<std::vector<int>>(height, std::vector<int>(width))}
        , foreground{std::vector<std::vector<int>>(height, std::vector<int>(width))}
    {}
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

Options handle_args(int argc, char* argv[])
{
    Options options{
        "output.map",
        true,
        20,
        20,
        false
    };

    for (int i = 1; i < argc; ++i) {
        auto raw_arg = std::string(argv[i]);

        if (raw_arg == "--new-file") {
            options.new_file = true;
            i++;
            options.filename = std::string(argv[i]);
        } else if (raw_arg == "--width") {
            i++;
            options.width = std::atoi(argv[i]);
        } else if (raw_arg == "--height") {
            i++;
            options.height = std::atoi(argv[i]);
        } else if (raw_arg == "--open-file") {
            options.open_file = true;
            i++;
            options.filename = std::string(argv[i]);
        } else {
            std::cout << "Unknown option: " << raw_arg << std::endl;
            std::cout << "Valid options:" << std::endl;
            std::cout << "  --new-file <filename>" << std::endl;
            std::cout << "  --width <value>" << std::endl;
            std::cout << "  --height <value>" << std::endl;
            std::cout << "  --open-file <filename>" << std::endl;
        }
    }

    return options;
}

struct MouseState 
{
    Vector2D<int> position;
    bool just_left_clicked;
    bool left_clicked;
};

class MapEditorWindow
{
public:
    MapEditorWindow(GameMap const& map, std::string const& map_filename)
        : camera_offset{0, 0}
        , map(map)
        , quit(false)
        , selected_section(BACKGROUND_SECTION)
        , selected_tile(-1)
        , map_filename(map_filename)
    {
        initialize_sdl();

        this->sdl_window = SDL_CreateWindow(
            "Pigs game - Map editor",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            1024,
            768,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );
        if (this->sdl_window == nullptr) {
            throw std::runtime_error("SDL Error: Window could not be created");
        }

        this->sdl_renderer = SDL_CreateRenderer(this->sdl_window, -1, SDL_RENDERER_ACCELERATED);
        SDL_SetRenderDrawBlendMode(this->sdl_renderer, SDL_BLENDMODE_BLEND);

        this->camera_offset.x = -int(this->map.width * TILE_SIZE / 2 - 200);
        this->camera_offset.y = int(this->map.height * TILE_SIZE / 2);

        auto load_spritesheet = [this](std::string const& filename) {
            return load_media("assets/sprites/" + filename, this->sdl_renderer);
        };
        this->tileset = load_spritesheet("tileset.png");
        this->foreground_set = load_spritesheet("foreground_set.png");
    }

    ~MapEditorWindow()
    {
        SDL_DestroyWindow(this->sdl_window);
        SDL_DestroyRenderer(this->sdl_renderer);
        SDL_Quit();
    }

    void run()
    {
        while (!this->quit) {
            this->handle_mouse_events();
            this->update_screen();
        }
    }

private:
    bool check_mouse_is_over(Region2D<int> region)
    {
        return check_aabb_collision(
            region.as<double>(),
            Region2D<int>{this->mouse.position.x, this->mouse.position.y, 0, 0}.as<double>()
        );
    }

    void handle_mouse_events()
    {
        SDL_Event e;
        this->mouse.just_left_clicked = false;
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT: {
                    this->quit = true;
                    break;
                }

                case SDL_MOUSEMOTION: {
                    if (e.motion.state & SDL_BUTTON_RMASK) {
                        this->camera_offset.x -= e.motion.xrel;
                        this->camera_offset.y += e.motion.yrel;
                    }
                    this->mouse.position.x = e.motion.x;
                    this->mouse.position.y = e.motion.y;
                    break;
                }

                case SDL_MOUSEBUTTONDOWN: {
                    this->mouse.just_left_clicked = true;
                    this->mouse.left_clicked = true;
                    break;
                }

                case SDL_MOUSEBUTTONUP: {
                    this->mouse.just_left_clicked = false;
                    this->mouse.left_clicked = false;
                    break;
                }
            }
        }
    }

    void update_screen()
    {
        SDL_SetRenderDrawColor(this->sdl_renderer, 63, 56, 81, 255);
        SDL_RenderClear(this->sdl_renderer);

        for (int i = 0; i < map.height; ++i) {
            for (int j = 0; j < map.width; ++j) {
                auto world_position = Vector2D<int>{TILE_SIZE * j, TILE_SIZE * (map.height - i - 1)};

                // Background
                {
                    auto tile_id = map.tilemap[i][j];
                    auto offset = Vector2D<int>{TILE_SIZE * (tile_id % 12), TILE_SIZE * int(floor(tile_id / 12))};
                    auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
                    draw_sprite(this->sdl_renderer, this->tileset, offset, world_position, size, camera_offset);
                }

                // Foreground
                {
                    auto tile_id = map.foreground[i][j];
                    auto offset = Vector2D<int>{TILE_SIZE * (tile_id % 7), TILE_SIZE * int(floor(tile_id / 7))};
                    auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
                    draw_sprite(this->sdl_renderer, this->foreground_set, offset, world_position, size, camera_offset);
                }

                // Check if selected
                {
                    auto camera_position = to_camera_position(world_position, {TILE_SIZE, TILE_SIZE}, camera_offset);
                    auto tile_region = Region2D<int>{camera_position.x, camera_position.y, TILE_SIZE, TILE_SIZE};
                    if (this->check_mouse_is_over(tile_region)) {
                        SDL_SetRenderDrawColor(this->sdl_renderer, 250, 255, 255, 255);
                        auto sdl_rect = to_sdl_rect(tile_region);
                        SDL_RenderDrawRect(this->sdl_renderer, &sdl_rect);

                        if (this->mouse.left_clicked && this->selected_tile != -1) {
                            if (selected_section == BACKGROUND_SECTION) {
                                this->map.tilemap[i][j] = this->selected_tile;
                            } else if (selected_section == FOREGROUND_SECTION) {
                                this->map.foreground[i][j] = this->selected_tile;
                            }
                        }
                    }
                }
            }
        }

        // Selected tile (mouse icon)
        {
            auto world_mouse = to_world_position(this->mouse.position, {0, 0}, camera_offset);
            world_mouse.x -= TILE_SIZE / 2;
            world_mouse.y -= TILE_SIZE / 2;
            auto size = Vector2D<int>{TILE_SIZE, TILE_SIZE};
            if (selected_section == BACKGROUND_SECTION) {
                auto offset = Vector2D<int>{TILE_SIZE * (selected_tile % 12), TILE_SIZE * int(floor(selected_tile / 12))};
                draw_sprite(this->sdl_renderer, tileset, offset, world_mouse, size, camera_offset);
            } else if (selected_section == FOREGROUND_SECTION) {
                auto offset = Vector2D<int>{TILE_SIZE * (selected_tile % 7), TILE_SIZE * int(floor(selected_tile / 7))};
                draw_sprite(this->sdl_renderer, foreground_set, offset, world_mouse, size, camera_offset);
            }
        }

        // Drawable region border
        {
            SDL_SetRenderDrawColor(this->sdl_renderer, 250, 200, 150, 255);
            auto map_size = Vector2D<int>{TILE_SIZE * map.width, TILE_SIZE * map.height};
            auto map_position = to_camera_position({0, 0}, map_size, camera_offset);
            auto map_rect = SDL_Rect{map_position.x, map_position.y, map_size.x, map_size.y};
            SDL_RenderDrawRect(this->sdl_renderer, &map_rect);
        }

        // Left panel
        {
            int window_w = 0;
            int window_h = 0;
            SDL_GetWindowSize(this->sdl_window, &window_w, &window_h);
            SDL_SetRenderDrawColor(this->sdl_renderer, 45, 35, 60, 255);
            auto fixed_rect = SDL_Rect{0, 0, 200, window_h};
            SDL_RenderFillRect(this->sdl_renderer, &fixed_rect);

            // Save button
            {
                auto save_btn_region = Region2D<int>{20, 5, 20, 20};

                SDL_SetRenderDrawColor(this->sdl_renderer, 0, 150, 0, 255);
                auto fixed_rect = to_sdl_rect(save_btn_region);
                SDL_RenderFillRect(this->sdl_renderer, &fixed_rect);

                if (this->check_mouse_is_over(save_btn_region)) {
                    SDL_SetRenderDrawColor(this->sdl_renderer, 0, 250, 0, 255);
                    SDL_RenderFillRect(this->sdl_renderer, &fixed_rect);

                    if (this->mouse.just_left_clicked) {
                        save_map(map, this->map_filename);
                        std::cout << "Map saved in " << this->map_filename << std::endl;
                    }
                }
            }

            // "Change tileset" button
            {
                auto change_tile_btn_region = Region2D<int>{45, 5, 20, 20};

                SDL_SetRenderDrawColor(this->sdl_renderer, 0, 0, 150, 255);
                auto fixed_rect = to_sdl_rect(change_tile_btn_region);
                SDL_RenderFillRect(this->sdl_renderer, &fixed_rect);

                if (this->check_mouse_is_over(change_tile_btn_region)) {
                    SDL_SetRenderDrawColor(this->sdl_renderer, 0, 0, 250, 255);
                    SDL_RenderFillRect(this->sdl_renderer, &fixed_rect);

                    if (this->mouse.just_left_clicked) {
                        if (this->selected_section == BACKGROUND_SECTION) {
                            this->selected_section = FOREGROUND_SECTION;
                        } else if (this->selected_section == FOREGROUND_SECTION) {
                            this->selected_section = BACKGROUND_SECTION;
                        }
                        this->selected_tile = -1;
                    }
                }
            }

            // "Fill" button
            {
                auto fill_btn_region = Region2D<int>{70, 5, 20, 20};

                SDL_SetRenderDrawColor(this->sdl_renderer, 150, 0, 0, 255);
                auto fixed_rect = to_sdl_rect(fill_btn_region);
                SDL_RenderFillRect(this->sdl_renderer, &fixed_rect);

                if (this->check_mouse_is_over(fill_btn_region)) {
                    SDL_SetRenderDrawColor(this->sdl_renderer, 250, 0, 0, 255);
                    SDL_RenderFillRect(this->sdl_renderer, &fixed_rect);

                    if (this->mouse.just_left_clicked) {
                        if (selected_section == BACKGROUND_SECTION) {
                            if (selected_tile != -1) {
                                for (int i = 0; i < map.height; ++i) {
                                    for (int j = 0; j < map.width; ++j) {
                                        map.tilemap[i][j] = selected_tile;
                                    }
                                }
                            }
                        } else if (selected_section == FOREGROUND_SECTION) {
                            if (selected_tile != -1) {
                                for (int i = 0; i < map.height; ++i) {
                                    for (int j = 0; j < map.width; ++j) {
                                        map.foreground[i][j] = selected_tile;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // TILES selection
            {
                if (selected_section == BACKGROUND_SECTION) {
                    draw_background_tileset(
                        this->sdl_renderer,
                        tileset,
                        this->mouse.left_clicked,
                        this->mouse.position,
                        selected_tile
                    );
                } else if (selected_section == FOREGROUND_SECTION) {
                    draw_foreground_tileset(
                        this->sdl_renderer,
                        foreground_set,
                        this->mouse.left_clicked,
                        this->mouse.position,
                        selected_tile
                    );
                }
            }

        }

        SDL_RenderPresent(this->sdl_renderer);        
    }

    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    Vector2D<int> camera_offset;
    GameMap map;
    MouseState mouse;
    bool quit;
    int selected_tile;
    int selected_section;
    SDL_Texture* tileset;
    SDL_Texture* foreground_set;
    std::string map_filename;
};

int main(int argc, char* argv[])
{
    auto options = handle_args(argc, argv);
    std::cout << "Filename: " << options.filename;
    std::cout << (options.new_file ? "(New file)" : "(loading existing file)") << std::endl;
    auto map = (options.open_file) ? load_map(options.filename) : GameMap(options.width, options.height);

    auto window = MapEditorWindow(map, options.filename);
    window.run();

    return 0;
}