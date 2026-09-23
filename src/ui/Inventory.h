#include "../player/Item.h"
#include "../basics/TextRenderer.h"
#include <optional>

void DrawInventory(int screenWidth, int screenHeight, std::optional<Item> hotbar[8], std::optional<Item> inventory[24], std::optional<Item> crafting[4], std::optional<Item> crafted, TextRenderer &textRenderer, int slotPressed, float slotSize = 160.0f, float padding = 20.0f);
int GetInventorySlotFromMouse(double mouseX, double mouseY, int screenWidth, int screenHeight, float slotSize = 160.0f, float padding = 20.0f);