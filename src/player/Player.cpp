#include "Player.h"
#include "ItemDatabase.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <GLFW/glfw3.h>

Player::Player(const PlayerStats &playerStats) : playerStats(playerStats), hotbarIndex(0)
{
}

std::vector<std::string> Player::GetItemsAsStrings()
{
    std::vector<std::string> strings;

    double currentTime = glfwGetTime();

    // Remove items older than 7 seconds
    itemsAdded.erase(
        std::remove_if(itemsAdded.begin(), itemsAdded.end(),
                       [currentTime](const ItemAdded &ia)
                       {
                           return (currentTime - ia.time) > 7.0;
                       }),
        itemsAdded.end());

    // Add valid items as strings
    for (const auto &ia : itemsAdded)
    {
        strings.push_back(std::to_string(ia.item.quantity) + "x " + ia.item.getItemName());
    }

    return strings;
}

int Player::AddItem(Item item)
{
    int originalSize = item.quantity;
    for (int i = 0; i < 8; ++i)
    {
        bool fill = false;
        if (hotbar[i])
        {
            if ((*hotbar[i]).itemID == item.itemID && (*hotbar[i]).quantity != ITEMS[item.itemID].maxStack)
            {
                fill = true;
            }
        }
        else
        {
            fill = true;
            hotbar[i] = Item(0, item.itemID);
        }

        if (fill)
        {
            int spaceLeft = ITEMS[item.itemID].maxStack - (*hotbar[i]).quantity;
            int removing = std::min(spaceLeft, item.quantity);
            (*hotbar[i]).quantity += removing;
            item.quantity -= removing;

            std::cout << "Added " + std::to_string(removing) + " of " + item.getItemName() + " to index " + std::to_string(i) << std::endl;

            if (item.quantity == 0)
            {
                break;
            }
        }
    }

    for (int i = 0; i < 24; ++i)
    {
        if (item.quantity == 0)
        {
            break;
        }

        bool fill = false;
        if (inventory[i])
        {
            if ((*inventory[i]).itemID == item.itemID && (*inventory[i]).quantity != ITEMS[item.itemID].maxStack)
            {
                fill = true;
            }
        }
        else
        {
            fill = true;
            inventory[i] = Item(0, item.itemID);
        }

        if (fill)
        {
            int spaceLeft = ITEMS[item.itemID].maxStack - (*inventory[i]).quantity;
            int removing = std::min(spaceLeft, item.quantity);
            (*inventory[i]).quantity += removing;
            item.quantity -= removing;

            std::cout << "Added " + std::to_string(removing) + " of " + item.getItemName() + " to index " + std::to_string(i) << " in inventory" << std::endl;
        }
    }

    if (originalSize != item.quantity)
    {
        Item pushItem = {originalSize - item.quantity, item.itemID};
        bool added = false;
        for (int i = 0; i < itemsAdded.size(); ++i)
        {
            if (itemsAdded[i].item.itemID == pushItem.itemID)
            {
                itemsAdded[i].item.quantity += pushItem.quantity;
                itemsAdded[i].time = glfwGetTime();
                std::rotate(itemsAdded.begin() + i, itemsAdded.begin() + i + 1, itemsAdded.end());
                added = true;
                break;
            }
        }
        if (!added)
        {
            itemsAdded.push_back({pushItem, glfwGetTime()});
        }
    }

    return item.quantity;
}

int Player::PlaceItem()
{
    if (hotbar[hotbarIndex])
    {
        Item item = *hotbar[hotbarIndex];
        if (item.quantity > 0)
        {
            int itemPlace = ITEMS[item.itemID].blockPlace;
            if (itemPlace != 0)
            {
                item.quantity -= 1;
                if (item.quantity == 0)
                {
                    hotbar[hotbarIndex] = std::nullopt;
                }
                else
                {
                    hotbar[hotbarIndex] = item;
                }
            }
            return itemPlace;
        }
    }
    return 0;
}

void SwapItemsPoint(std::optional<Item> &item1, std::optional<Item> &item2)
{
    bool done = false;
    if (item1 && item2)
    {
        if ((*item1).itemID == (*item2).itemID)
        {
            int id = (*item1).itemID;
            int maxStack = ITEMS[id].maxStack;
            int totalQuantity = (*item1).quantity + (*item2).quantity;
            if (totalQuantity > maxStack)
            {
                Item item1P = {totalQuantity - maxStack, id};
                Item item2P = {maxStack, id};
                item1 = item1P;
                item2 = item2P;
            }
            else
            {
                item1 = std::nullopt;
                Item item = {totalQuantity, id};
                item2 = item;
            }
            done = true;
        }
    }
    if (!done)
    {
        std::swap(item1, item2);
    }
}

void Player::SwapItems(int item1, int item2)
{
    std::optional<Item> *combined[24 + 8 + 4 + 1]; // 37 if using hotbar[8]

    // Fill combined array with pointers:
    int index = 0;
    for (int i = 0; i < 24; ++i)
        combined[index++] = &inventory[i];
    for (int i = 0; i < 8; ++i)
        combined[index++] = &hotbar[i];
    for (int i = 0; i < 4; ++i)
        combined[index++] = &crafting[i];
    combined[36] = &craftedItem;
    SwapItemsPoint(*combined[item1], *combined[item2]);

    // if (item1 >= 0 && item1 != 36)
    // {
    //     if (item2 >= 0 && item2 < 24)
    //     {
    //         if (item1 >= 0 && item1 < 24)
    //         {
    //             SwapItemsPoint(inventory[item1], inventory[item2]);
    //         }
    //         else if (item1 < 32)
    //         {
    //             SwapItemsPoint(hotbar[item1 - 24], inventory[item2]);
    //         }
    //         else
    //         {
    //             SwapItemsPoint(crafting[item1 - 32], inventory[item2]);
    //         }
    //     }
    //     else if (item2 >= 24 && item2 < 32)
    //     {
    //         if (item1 >= 0 && item1 < 24)
    //         {
    //             SwapItemsPoint(inventory[item1], hotbar[item2 - 24]);
    //         }
    //         else if (item1 < 32)
    //         {
    //             SwapItemsPoint(hotbar[item1 - 24], hotbar[item2 - 24]);
    //         }
    //         else
    //         {
    //             SwapItemsPoint(crafting[item1 - 32], hotbar[item2 - 24]);
    //         }
    //     }
    //     else if (item2 >= 32)
    //     {
    //         if (item1 >= 0 && item1 < 24)
    //         {
    //             SwapItemsPoint(inventory[item1], crafting[item2 - 32]);
    //         }
    //         else if (item1 < 32)
    //         {
    //             SwapItemsPoint(hotbar[item1 - 24], crafting[item2 - 32]);
    //         }
    //         else
    //         {
    //             SwapItemsPoint(crafting[item1 - 32], crafting[item2 - 32]);
    //         }
    //     }
    // }
    // else if (item1 == 36)
    // {
    //     if (item2 >= 0 && item2 < 24)
    //     {
    //         SwapItemsPoint(craftedItem, inventory[item2]);
    //     }
    //     else if (item2 >= 24 && item2 < 32)
    //     {
    //         SwapItemsPoint(craftedItem, hotbar[item2 - 24]);
    //     }
    //     else if (item2 >= 32)
    //     {
    //         SwapItemsPoint(craftedItem, crafting[item2 - 32]);
    //     }
    // }
}

bool Player::AddItemSpecific(int item1, int item2)
{
    std::optional<Item> *combined[24 + 8 + 4]; // 37 if using hotbar[8]

    // Fill combined array with pointers:
    int index = 0;
    for (int i = 0; i < 24; ++i)
        combined[index++] = &inventory[i];
    for (int i = 0; i < 8; ++i)
        combined[index++] = &hotbar[i];
    for (int i = 0; i < 4; ++i)
        combined[index++] = &crafting[i];

    bool success = true;
    if (combined[item1] && combined[item1]->has_value())
    {
        if (combined[item2] && combined[item2]->has_value())
        {
            if ((*combined[item2])->itemID == (*combined[item1])->itemID)
            {
                *combined[item2] = Item{(*combined[item2])->quantity + 1, (*combined[item2])->itemID};
            }
            else
            {
                success = false;
            }
        }
        else
        {
            *combined[item2] = Item{1, (*combined[item1])->itemID};
        }

        if (success)
        {
            *combined[item1] = Item{(*combined[item1])->quantity - 1, (*combined[item1])->itemID};
            if ((*combined[item1])->quantity == 0)
            {
                *combined[item1] = std::nullopt;
                return true;
            }
        }
    }
    return false;

    // if (item1 >= 0 && item1 < 24)
    // {
    //     if (inventory[item1])
    //     {
    //         bool done = false;
    //         if (item2)
    //         {
    //         }
    //         else
    //         {
    //         }

    //         (*inventory[item1]).quantity -= 1;
    //         if ((*inventory[item1]).quantity == 0)
    //         {
    //             inventory[item1] = std::nullopt;
    //         }
    //     }
    // }
    // else if (item1 >= 24 && item1 < 32)
    // {
    // }
    // else if (item1 >= 32 && item1 < 36)
    // {
    // }
    // else if (item1 == 36)
    // {
    // }
}

void Player::RemoveCraftingLayer()
{
    for (int i = 0; i < 4; ++i)
    {
        if (crafting[i])
        {
            Item item = *crafting[i];
            item.quantity -= 1;
            if (item.quantity == 0)
            {
                crafting[i] = std::nullopt;
            }
            else
            {
                crafting[i] = item;
            }
        }
    }
}
