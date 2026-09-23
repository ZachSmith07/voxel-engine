#include <optional>
#include "Item.h"

struct Recipe
{
    std::optional<int> items[9];
    Item item;
};

static Recipe RECIPES[] = {
    { { 4, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt }, { 4, 6 } },
    { { 6, 6, std::nullopt, 6, 6, std::nullopt, std::nullopt, std::nullopt, std::nullopt }, { 1, 7 } }
};
