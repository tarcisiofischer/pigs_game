#include <io.hpp>
#include <logging.hpp>

void save_map(GameMap const& map, std::string const& filename)
{
    std::ofstream mapfile(filename, std::ios::binary | std::ios::out);
    if (!mapfile.is_open()) {
        err("Could not open file to write. filename="s + filename);
    }
    auto bin_write = [&mapfile](int const& data) { mapfile.write(reinterpret_cast<const char*>(&data), sizeof(int)); };

    bin_write(map.width);
    bin_write(map.height);
    for (int i = 0; i < map.height; ++i) {
        for (int j = 0; j < map.width; ++j) {
            bin_write(map.tilemap[i][j]);
        }
    }
    bin_write(map.interactables.size());
    for (auto const& interactable : map.interactables) {
        bin_write(interactable.position.x);
        bin_write(interactable.position.y);
        bin_write(interactable.id);
        bin_write(interactable.flip);
    }
    mapfile.close();
}

GameMap load_map(std::string const& filename)
{
    std::ifstream mapfile(filename, std::ios::binary | std::ios::in);
    if (!mapfile.is_open()) {
        err("Could not load file to read. filename="s + filename);
    }

    auto bin_read_nextint = [&mapfile](int& target) { mapfile.read(reinterpret_cast<char*>(&target), sizeof(int)); };

    auto map = GameMap { 0, 0 };

    bin_read_nextint(map.width);
    bin_read_nextint(map.height);

    map.tilemap = std::vector<std::vector<int>>(map.height, std::vector<int>(map.width));
    for (int i = 0; i < map.height; ++i) {
        for (int j = 0; j < map.width; ++j) {
            bin_read_nextint(map.tilemap[i][j]);
        }
    }

    int size = 0;
    bin_read_nextint(size);
    int position_x = 0;
    int position_y = 0;
    int id = 0;
    int flip = 0;
    for (int i = 0; i < size; ++i) {
        bin_read_nextint(position_x);
        bin_read_nextint(position_y);
        bin_read_nextint(id);
        bin_read_nextint(flip);
        map.interactables.push_back({ { position_x, position_y }, id, flip });
    }

    mapfile.close();

    return map;
}
