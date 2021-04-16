#ifndef __IO_H
#define __IO_H

#include <GameMap.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

void save_map(GameMap const& map, std::string const& filename);

GameMap load_map(std::string const& filename);

#endif
