#include "Item.h"
#include "ItemDatabase.h"

Item::Item(int quantity, int itemID) : quantity(quantity), itemID(itemID)
{

}

std::string Item::getItemName() const
{
    return ITEMS[itemID].name;
}
