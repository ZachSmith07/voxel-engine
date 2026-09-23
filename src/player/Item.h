#pragma once
#include <string>
class Item
{
    public:
        Item(int quantity, int itemID);

        int quantity;
        int itemID;

        std::string getItemName() const;
};