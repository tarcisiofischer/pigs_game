#ifndef __IO_H
#define __IO_H

#include <GameMap.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void save_map(GameMap const& map, std::string const& filename);

GameMap load_map(std::string const& filename);

#endif
