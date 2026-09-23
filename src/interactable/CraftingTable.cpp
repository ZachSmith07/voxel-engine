#include "CraftingTable.h"

CraftingTable::CraftingTable()
{

}

CraftingTable::CraftingTable(std::optional<Item> itemsE[9])
{
    std::copy(itemsE, itemsE + 9, items);
}
