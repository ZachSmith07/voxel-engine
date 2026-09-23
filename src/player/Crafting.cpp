#include "Crafting.h"
#include "Recipes.h"

std::optional<Item> CalculateItem(std::optional<Item> craftingItems[4])
{
    std::optional<Item> craftItems[9] = {craftingItems[0], craftingItems[1], std::nullopt, craftingItems[2], craftingItems[3], std::nullopt, std::nullopt, std::nullopt, std::nullopt};
    for (int i = 0; i < 3; ++i)
    {
        if (!craftItems[0] && !craftItems[3] && !craftItems[6])
        {
            craftItems[0] = craftItems[1];
            craftItems[1] = craftItems[2];
            craftItems[2] = std::nullopt;
            craftItems[3] = craftItems[4];
            craftItems[4] = craftItems[5];
            craftItems[5] = std::nullopt;
            craftItems[6] = craftItems[7];
            craftItems[7] = craftItems[8];
            craftItems[8] = std::nullopt;
        }
        else
        {
            break;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        if (!craftItems[0] && !craftItems[1] && !craftItems[2])
        {
            craftItems[0] = craftItems[3];
            craftItems[1] = craftItems[4];
            craftItems[2] = craftItems[5];
            craftItems[3] = craftItems[6];
            craftItems[4] = craftItems[7];
            craftItems[5] = craftItems[8];
            craftItems[6] = std::nullopt;
            craftItems[7] = std::nullopt;
            craftItems[8] = std::nullopt;
        }
        else
        {
            break;
        }
    }
    return CalculateItem9(craftItems);
}

std::optional<Item> CalculateItem9(std::optional<Item> craftingItems[9])
{
    // std::optional<Item> craftItems[9] = {craftingItems[0], craftingItems[1], std::nullopt, craftingItems[2], craftingItems[3], std::nullopt, std::nullopt, std::nullopt, std::nullopt};
    for (Recipe recipe : RECIPES)
    {
        bool correct = true;
        for (int i = 0; i < 9; ++i)
        {
            if (craftingItems[i])
            {
                if (recipe.items[i])
                {
                    if ((*craftingItems[i]).itemID == (*recipe.items[i]))
                    {
                        // CORRECT
                    }
                    else
                    {
                        correct = false;
                        break;
                    }
                }
                else
                {
                    correct = false;
                    break;
                }
            }
            else
            {
                if (recipe.items[i])
                {
                    correct = false;
                    break;
                }
            }
        }

        if (correct)
        {
            return recipe.item;
        }
    }

    return std::nullopt;
}
