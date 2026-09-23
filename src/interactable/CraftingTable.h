#pragma once
#include "../player/Item.h"
#include <optional>

class CraftingTable
{
    public:
        CraftingTable();
        CraftingTable(std::optional<Item> itemsE[9]);
        std::optional<Item> items[9];
        std::optional<Item> craftedItem;
};