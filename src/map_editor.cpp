#include <SDL.h>

#include <GameMap.hpp>
#include <Vector2D.hpp>
#include <collision/aabb.hpp>
#include <constants.hpp>
#include <drawing.hpp>
#include <functional>
#include <io.hpp>
#include <iostream>
#include <sdl_wrappers.hpp>
#include <string>
#include <vector>

auto constexpr BACKGROUND_SECTION = 1;
auto constexpr FOREGROUND_SECTION = 2;
auto constexpr INTERACTABLES_SECTION = 3;

auto constexpr LIGHT_GRAY_COLOR = RGBColor { 102, 102, 102 };
auto constexpr GRAY_COLOR = RGBColor { 72, 72, 72 };
auto constexpr DARK_PURPLE_COLOR = RGBColor { 45, 35, 60 };
auto constexpr PURPLE_COLOR = RGBColor { 63, 56, 81 };

struct Options {
    std::string filename;

    bool new_file;
    int width;
    int height;

    bool open_file;
};

Options handle_args(int argc, char* argv[])
{
    Options options { "output.map", true, 20, 20, false };

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
            options.new_file = false;
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

struct MouseState {
    Vector2D<int> position;
    bool just_left_clicked;
    bool just_right_clicked;
    bool left_clicked;
    bool right_clicked;
};

bool check_mouse_is_over(MouseState const& mouse, Region2D<int> region)
{
    return check_aabb_collision(region.as<double>(),
        Region2D<int> { mouse.position.x, mouse.position.y, 0, 0 }.as<double>());
}

class Button {
public:
    Button()
    {
    }

    Button(SDL_Renderer* renderer, Vector2D<int> const& position, Vector2D<int> const& size, RGBColor const& color,
        std::string const& filename)
        : color(color)
        , sdl_renderer(renderer)
    {
        this->sdl_image = load_media(filename, this->sdl_renderer);
        this->region = Region2D<int> { position.x, position.y, size.x, size.y };
        this->sdl_region = to_sdl_rect(this->region);
    }

    void update(MouseState const& mouse)
    {
        if (check_mouse_is_over(mouse, this->region)) {
            if (this->on_mouse_in && !this->mouse_is_over) {
                (*this->on_mouse_in)(*this, mouse);
            }

            if (this->on_mouse_clicked && mouse.just_left_clicked) {
                (*this->on_mouse_clicked)(*this, mouse);
            }

            this->mouse_is_over = true;
        } else {
            if (this->on_mouse_out && this->mouse_is_over) {
                (*this->on_mouse_out)(*this, mouse);
            }

            this->mouse_is_over = false;
        }
    }

    void draw()
    {
        SDL_SetRenderDrawColor(this->sdl_renderer, this->color.r, this->color.g, this->color.b, 255);
        SDL_RenderFillRect(this->sdl_renderer, &this->sdl_region);

        auto srcrect = SDL_Rect { 0, 0, this->sdl_region.w, this->sdl_region.h };
        SDL_RenderCopy(this->sdl_renderer, this->sdl_image, &srcrect, &this->sdl_region);
    }

    void register_on_mouse_in(std::function<void(Button& self, MouseState const&)> const& f)
    {
        this->on_mouse_in = f;
    }

    void register_on_mouse_out(std::function<void(Button& self, MouseState const&)> const& f)
    {
        this->on_mouse_out = f;
    }

    void register_on_mouse_clicked(std::function<void(Button& self, MouseState const&)> const& f)
    {
        this->on_mouse_clicked = f;
    }

    void set_color(RGBColor const& c)
    {
        this->color = c;
    }

private:
    Region2D<int> region;
    RGBColor color;

    SDL_Rect sdl_region;
    SDL_Renderer* sdl_renderer;
    SDL_Texture* sdl_image;

    bool mouse_is_over;
    std::optional<std::function<void(Button& self, MouseState const&)>> on_mouse_in;
    std::optional<std::function<void(Button& self, MouseState const&)>> on_mouse_out;
    std::optional<std::function<void(Button& self, MouseState const&)>> on_mouse_clicked;
};

class MapEditorWindow {
public:
    MapEditorWindow(GameMap const& map, std::string const& map_filename)
        : camera_offset { 0, 0 }
        , map(map)
        , mouse { { 0, 0 }, false, false, false }
        , quit(false)
        , selected_section(BACKGROUND_SECTION)
        , selected_tile(-1)
        , map_filename(map_filename)
        , bottom_panel_message("")
    {
        this->sdl_window = SDL_CreateWindow("Pigs game - Map editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            1200, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
        this->interactables_set = load_media("assets/map_editor/interactables.png", this->sdl_renderer);
        this->monogram = load_spritesheet("monogram.png");

        this->new_button = Button(this->sdl_renderer, { 20, 5 }, { 22, 22 }, GRAY_COLOR, "assets/map_editor/new.png");
        this->new_button.register_on_mouse_in(
            [this](Button& self, MouseState const& mouse) { self.set_color(LIGHT_GRAY_COLOR); });
        this->new_button.register_on_mouse_out(
            [this](Button& self, MouseState const& mouse) { self.set_color(GRAY_COLOR); });

        this->load_button = Button(this->sdl_renderer, { 44, 5 }, { 22, 22 }, GRAY_COLOR, "assets/map_editor/load.png");
        this->load_button.register_on_mouse_in(
            [this](Button& self, MouseState const& mouse) { self.set_color(LIGHT_GRAY_COLOR); });
        this->load_button.register_on_mouse_out(
            [this](Button& self, MouseState const& mouse) { self.set_color(GRAY_COLOR); });

        this->save_button = Button(this->sdl_renderer, { 68, 5 }, { 22, 22 }, GRAY_COLOR, "assets/map_editor/save.png");
        this->save_button.register_on_mouse_in(
            [this](Button& self, MouseState const& mouse) { self.set_color(LIGHT_GRAY_COLOR); });
        this->save_button.register_on_mouse_out(
            [this](Button& self, MouseState const& mouse) { self.set_color(GRAY_COLOR); });
        this->save_button.register_on_mouse_clicked([this](Button& self, MouseState const& mouse) {
            save_map(this->map, this->map_filename);
            this->bottom_panel_message = "Map saved in " + this->map_filename;
        });

        this->tm0 = Button(this->sdl_renderer, { 20, 460 }, { 14, 14 }, PURPLE_COLOR, "assets/map_editor/tm0.png");
        this->tm0.register_on_mouse_clicked(
            [this](Button&, MouseState const&) { this->selected_section = BACKGROUND_SECTION; });
        this->tm1 = Button(this->sdl_renderer, { 36, 460 }, { 14, 14 }, PURPLE_COLOR, "assets/map_editor/tm1.png");
        this->tm1.register_on_mouse_clicked(
            [this](Button&, MouseState const&) { this->selected_section = FOREGROUND_SECTION; });
        this->tmI = Button(this->sdl_renderer, { 52, 460 }, { 14, 14 }, PURPLE_COLOR, "assets/map_editor/tmI.png");
        this->tmI.register_on_mouse_clicked(
            [this](Button&, MouseState const&) { this->selected_section = INTERACTABLES_SECTION; });

        this->fill_all_button = Button(this->sdl_renderer, { 134, 460 }, { 14, 14 }, PURPLE_COLOR, "assets/map_editor/fill_all.png");
        this->fill_all_button.register_on_mouse_clicked([this](Button&, MouseState const&) {
            if (mouse.just_left_clicked && this->selected_tile != -1 && (this->selected_section == BACKGROUND_SECTION || this->selected_section == FOREGROUND_SECTION)) {
                auto* selected_tilemap = ([this]() {
                    if (this->selected_section == BACKGROUND_SECTION) {
                        return &this->map.tilemap;
                    } else {
                        if (this->selected_section != FOREGROUND_SECTION) {
                            throw std::runtime_error("Unknown selected_section state");
                        }
                        return &this->map.foreground;
                    }
                })();

                for (int i = 0; i < this->map.height; ++i) {
                    for (int j = 0; j < this->map.width; ++j) {
                        (*selected_tilemap)[i][j] = this->selected_tile;
                    }
                }
            }
        });

        this->start_tile_id = 0;
        this->left_arrow_button = Button(this->sdl_renderer, { 152, 460 }, { 14, 14 }, PURPLE_COLOR, "assets/map_editor/left_arrow.png");
        this->left_arrow_button.register_on_mouse_clicked([this](Button&, MouseState const&) {
            if (this->start_tile_id > 0) {
                this->start_tile_id -= 1;
            }
        });
        this->right_arrow_button = Button(this->sdl_renderer, { 168, 460 }, { 14, 14 }, PURPLE_COLOR, "assets/map_editor/right_arrow.png");
        this->right_arrow_button.register_on_mouse_clicked(
            [this](Button&, MouseState const&) { this->start_tile_id += 1; });
    }

    ~MapEditorWindow()
    {
        SDL_DestroyWindow(this->sdl_window);
        SDL_DestroyRenderer(this->sdl_renderer);
    }

    void run()
    {
        while (!this->quit) {
            this->handle_mouse_events();
            this->update_components();
            this->draw_screen();
        }
    }

private:
    void update_components()
    {
        this->new_button.update(this->mouse);
        this->load_button.update(this->mouse);
        this->save_button.update(this->mouse);

        this->tm0.update(this->mouse);
        this->tm1.update(this->mouse);
        this->tmI.update(this->mouse);
        this->fill_all_button.update(this->mouse);
        this->left_arrow_button.update(this->mouse);
        this->right_arrow_button.update(this->mouse);
    }

    bool check_mouse_is_over(Region2D<int> region)
    {
        return check_aabb_collision(region.as<double>(),
            Region2D<int> { this->mouse.position.x, this->mouse.position.y, 0, 0 }.as<double>());
    }

    void handle_mouse_events()
    {
        SDL_Event e;
        this->mouse.just_left_clicked = false;
        this->mouse.just_right_clicked = false;
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
                if (e.button.button == SDL_BUTTON_LEFT) {
                    this->mouse.just_left_clicked = true;
                    this->mouse.left_clicked = true;
                }
                if (e.button.button == SDL_BUTTON_RIGHT) {
                    this->mouse.right_clicked = true;
                    this->mouse.just_right_clicked = true;
                }
                break;
            }

            case SDL_MOUSEBUTTONUP: {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    this->mouse.just_left_clicked = false;
                    this->mouse.left_clicked = false;
                }
                if (e.button.button == SDL_BUTTON_RIGHT) {
                    this->mouse.right_clicked = false;
                    this->mouse.just_right_clicked = false;
                }
                break;
            }
            }
        }
    }

    void draw_screen()
    {
        SDL_SetRenderDrawColor(this->sdl_renderer, 63, 56, 81, 255);
        SDL_RenderClear(this->sdl_renderer);

        this->draw_main_region();
        this->draw_mouse();
        this->draw_left_panel();
        this->draw_top_panel();
        this->draw_bottom_panel();

        SDL_RenderPresent(this->sdl_renderer);
    }

    void draw_main_region()
    {
        for (int i = 0; i < map.height; ++i) {
            for (int j = 0; j < map.width; ++j) {
                auto world_position = Vector2D<int> { TILE_SIZE * j, TILE_SIZE * (map.height - i - 1) };

                // Background
                {
                    auto tile_id = map.tilemap[i][j];
                    auto offset = Vector2D<int> { TILE_SIZE * (tile_id % 12), TILE_SIZE * int(floor(tile_id / 12)) };
                    auto size = Vector2D<int> { TILE_SIZE, TILE_SIZE };
                    draw_sprite(this->sdl_renderer, this->tileset, offset, world_position, size, camera_offset);
                }

                // Foreground
                {
                    auto tile_id = map.foreground[i][j];
                    auto offset = Vector2D<int> { TILE_SIZE * (tile_id % 7), TILE_SIZE * int(floor(tile_id / 7)) };
                    auto size = Vector2D<int> { TILE_SIZE, TILE_SIZE };
                    draw_sprite(this->sdl_renderer, this->foreground_set, offset, world_position, size, camera_offset);
                }
            }
        }

        // Interactables
        {
            auto size = Vector2D<int> { TILE_SIZE, TILE_SIZE };
            for (auto const& interactable_info : this->map.interactables) {
                auto offset = Vector2D<int> { TILE_SIZE * (interactable_info.id % 7),
                    TILE_SIZE * int(floor(interactable_info.id / 7)) };
                auto world_position = interactable_info.position;
                draw_sprite(this->sdl_renderer, this->interactables_set, offset, world_position, size, camera_offset);
            }
        }

        for (int i = 0; i < map.height; ++i) {
            for (int j = 0; j < map.width; ++j) {
                auto world_position = Vector2D<int> { TILE_SIZE * j, TILE_SIZE * (map.height - i - 1) };

                // Check if selected
                {
                    auto camera_position = to_camera_position(world_position, { TILE_SIZE, TILE_SIZE }, camera_offset);
                    auto tile_region = Region2D<int> { camera_position.x, camera_position.y, TILE_SIZE, TILE_SIZE };
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
                        if (this->mouse.just_left_clicked && this->selected_tile != -1) {
                            if (selected_section == INTERACTABLES_SECTION) {
                                // TODO: check how to flip
                                this->map.interactables.push_back({ world_position, this->selected_tile, +1 });
                            }
                        }

                        if (this->mouse.just_right_clicked) {
                            if (selected_section == INTERACTABLES_SECTION) {
                                this->map.interactables.erase(
                                    std::remove_if(
                                        this->map.interactables.begin(), this->map.interactables.end(),
                                        [&world_position](InteractableInfo const& interactable_info) {
                                            return (fabs(interactable_info.position.x - world_position.x) <= 1e-4 && fabs(interactable_info.position.y - world_position.y) <= 1e-4);
                                        }),
                                    this->map.interactables.end());
                            }
                        }
                    }
                }
            }
        }

        // Drawable region border
        {
            SDL_SetRenderDrawColor(this->sdl_renderer, 250, 200, 150, 255);
            auto map_size = Vector2D<int> { TILE_SIZE * map.width, TILE_SIZE * map.height };
            auto map_position = to_camera_position({ 0, 0 }, map_size, camera_offset);
            auto map_rect = SDL_Rect { map_position.x, map_position.y, map_size.x, map_size.y };
            SDL_RenderDrawRect(this->sdl_renderer, &map_rect);
        }
    }

    void draw_mouse()
    {
        auto world_mouse = to_world_position(this->mouse.position, { 0, 0 }, camera_offset);
        world_mouse.x -= TILE_SIZE / 2;
        world_mouse.y -= TILE_SIZE / 2;
        auto size = Vector2D<int> { TILE_SIZE, TILE_SIZE };
        if (this->selected_section == BACKGROUND_SECTION) {
            auto offset = Vector2D<int> { TILE_SIZE * (selected_tile % 12), TILE_SIZE * int(floor(selected_tile / 12)) };
            draw_sprite(this->sdl_renderer, this->tileset, offset, world_mouse, size, camera_offset);
        } else if (this->selected_section == FOREGROUND_SECTION) {
            auto offset = Vector2D<int> { TILE_SIZE * (selected_tile % 7), TILE_SIZE * int(floor(selected_tile / 7)) };
            draw_sprite(this->sdl_renderer, this->foreground_set, offset, world_mouse, size, camera_offset);
        } else if (this->selected_section == INTERACTABLES_SECTION) {
            auto offset = Vector2D<int> { TILE_SIZE * (selected_tile % 7), TILE_SIZE * int(floor(selected_tile / 7)) };
            draw_sprite(this->sdl_renderer, this->interactables_set, offset, world_mouse, size, camera_offset);
        }
    }

    void draw_top_panel()
    {
        int window_w = 0;
        int window_h = 0;
        SDL_GetWindowSize(this->sdl_window, &window_w, &window_h);
        draw_filled_region(this->sdl_renderer, { 0, 0, window_w, 30 }, GRAY_COLOR);
        draw_line(this->sdl_renderer, { 0, 30 }, { window_w, 30 }, DARK_PURPLE_COLOR);

        this->new_button.draw();
        this->load_button.draw();
        this->save_button.draw();
    }

    void draw_bottom_panel()
    {
        int window_w = 0;
        int window_h = 0;
        SDL_GetWindowSize(this->sdl_window, &window_w, &window_h);
        SDL_SetRenderDrawColor(this->sdl_renderer, 90, 80, 100, 255);
        auto fixed_rect = SDL_Rect { 0, window_h - 20, window_w, 20 };
        SDL_RenderFillRect(this->sdl_renderer, &fixed_rect);

        auto mouse_on_world = to_world_position(this->mouse.position, { 0, 0 }, camera_offset);
        auto message = this->map_filename + " [" + std::to_string(this->map.width) + "x" + std::to_string(this->map.height) + "]"
                                                                                                                              " . Mouse position: "
            + std::to_string(mouse_on_world.x) + ", " + std::to_string(mouse_on_world.y) + " . " + this->bottom_panel_message;
        gout(this->sdl_renderer, this->monogram, { 5, window_h - 15 }, message, { 255, 255, 255 });
    }

    void draw_left_panel()
    {
        int window_w = 0;
        int window_h = 0;
        SDL_GetWindowSize(this->sdl_window, &window_w, &window_h);
        draw_filled_region(this->sdl_renderer, { 0, 0, 200, window_h }, DARK_PURPLE_COLOR);

        // Tiles tab
        draw_filled_region(this->sdl_renderer, { 10, 50, 180, 430 }, PURPLE_COLOR);
        auto constexpr MAX_X = 4;
        auto constexpr MAX_Y = 9;
        auto constexpr MAX_ELEMS = MAX_X * MAX_Y;

        auto* selected_tileset = ([this]() -> SDL_Texture* {
            if (this->selected_section == BACKGROUND_SECTION) {
                return this->tileset;
            } else if (this->selected_section == FOREGROUND_SECTION) {
                return this->foreground_set;
            } else if (this->selected_section == INTERACTABLES_SECTION) {
                return this->interactables_set;
            }
            return nullptr;
        })();

        int w = 0;
        int h = 0;
        SDL_QueryTexture(selected_tileset, nullptr, nullptr, &w, &h);
        auto n_tiles_per_line = w / TILE_SIZE;

        auto x_offset = 0;
        auto y_offset = 0;
        for (int i = 0; i < MAX_ELEMS; ++i) {
            x_offset = 10 + i % MAX_X * (TILE_SIZE + 10);
            y_offset = 10 + int(i / MAX_X) * (TILE_SIZE + 10);

            auto tile_offset_i = (this->start_tile_id + i % MAX_X) * TILE_SIZE;
            auto tile_offset_j = (i / MAX_X) * TILE_SIZE;

            auto tile_id = this->start_tile_id + ((i / MAX_X) * n_tiles_per_line) + (i % MAX_X);

            auto tile_position = Vector2D<int> { 10 + x_offset, 50 + y_offset };
            draw_direct_sprite(this->sdl_renderer, selected_tileset, { tile_offset_i, tile_offset_j },
                { tile_position.x, tile_position.y }, { TILE_SIZE, TILE_SIZE });

            for (auto const& [di, dj] : std::array<std::tuple<int, int>, 4> { { { +1, 0 }, { -1, 0 }, { 0, +1 }, { 0, -1 } } }) {
                gout(this->sdl_renderer, this->monogram, { 10 + x_offset + 2 + di, 50 + y_offset + 2 + dj },
                    std::to_string(tile_id), { 255, 255, 255 });
            }
            gout(this->sdl_renderer, this->monogram, { 10 + x_offset + 2, 50 + y_offset + 2 }, std::to_string(tile_id),
                DARK_PURPLE_COLOR);

            // Draw selected tile border (if selected)
            auto tile_region = Region2D<int> { tile_position.x, tile_position.y, TILE_SIZE, TILE_SIZE };
            auto mouse_is_over = check_aabb_collision(tile_region.as<double>(),
                Region2D<int> { this->mouse.position.x, this->mouse.position.y, 0, 0 }.as<double>());
            auto tile_is_selected = selected_tile == tile_id;
            if (mouse_is_over || tile_is_selected) {
                SDL_SetRenderDrawColor(this->sdl_renderer, 255, 255, 255, 255);
                auto sdl_rect = to_sdl_rect(tile_region);
                SDL_RenderDrawRect(this->sdl_renderer, &sdl_rect);

                if (this->mouse.just_left_clicked) {
                    selected_tile = tile_id;
                }
            }
        }
        this->tm0.draw();
        this->tm1.draw();
        this->tmI.draw();
        this->left_arrow_button.draw();
        this->right_arrow_button.draw();
        this->fill_all_button.draw();
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
    SDL_Texture* interactables_set;
    SDL_Texture* monogram;
    std::string map_filename;
    std::string bottom_panel_message;

    Button new_button;
    Button load_button;
    Button save_button;

    int start_tile_id;
    Button left_arrow_button;
    Button right_arrow_button;
    Button fill_all_button;
    Button tm0;
    Button tm1;
    Button tmI;
};

int main(int argc, char* argv[])
{
    SDL_Handler _();

    auto options = handle_args(argc, argv);
    std::cout << "Filename: " << options.filename;
    std::cout << (options.new_file ? "(New file)" : "(loading existing file)") << std::endl;
    auto map = (options.open_file) ? load_map(options.filename) : GameMap(options.width, options.height);

    auto window = MapEditorWindow(map, options.filename);
    window.run();

    return 0;
}