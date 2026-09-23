#pragma once

#include "PlayerStats.h"
#include "Item.h"
#include <optional>
#include "ItemsAdded.h"
#include <vector>

class Player
{
    public:
        Player(const PlayerStats& playerStats);

        PlayerStats playerStats;
        std::optional<Item> hotbar[8];
        std::optional<Item> inventory[24];
        std::optional<Item> crafting[4];
        std::optional<Item> craftedItem;
        int hotbarIndex;

        std::vector<ItemAdded> itemsAdded;

        std::vector<std::string> GetItemsAsStrings();

        int AddItem(Item item);
        int PlaceItem();
        void SwapItems(int item1, int item2);
        void RemoveCraftingLayer();
        bool AddItemSpecific(int item1, int item2);

};