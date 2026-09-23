#include "../player/Item.h"
#include "../basics/TextRenderer.h"
#include <optional>

void DrawHotbar(int screenWidth, int screenHeight, std::optional<Item> items[8], TextRenderer &textRenderer, int hotbarIndex=0, int numSlots = 8, float slotSize = 160.0f, float padding = 35.0f);