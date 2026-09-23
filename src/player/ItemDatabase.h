#pragma once
#include <string>
#include "../basics/TileCoord.h"
#include <functional>
#include <glm/glm.hpp>

struct ItemData
{
    std::string name;
    int maxStack;
    TileCoord tileCoord;
    int blockPlace;
};


static ItemData ITEMS[] = {
    {"Grass", 4, {1, 0}, 1},
    {"Dirt", 36, {0, 0}, 2},
    {"Stone", 36, {2, 0}, 3},
    {"Sand", 36, {3, 0}, 4},
    {"Oak Log", 36, {4, 0}, 12},
    {"Leaves", 36, {5, 0}, 13},
    {"Oak Planks", 36, {6, 0}, 14},
    {"Crafting Table", 4, {7, 0}, 15},
};