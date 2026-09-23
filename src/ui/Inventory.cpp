#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Basics.h"
#include "../player/ItemDatabase.h"
#include "Inventory.h"
#include "../player/Crafting.h"

extern unsigned int textureID;

void DrawItemWindow(std::optional<Item> item, float x, float y, float slotSize, TextRenderer &textRenderer, bool slotPressed, glm::vec3 clickColour = {0.8f, 0.8f, 1.0f})
{
    if (slotPressed)
    {
        glColor4f(clickColour.x, clickColour.y, clickColour.z, 1.0f);
    }
    else
    {
        glColor4f(0.65f, 0.65f, 0.65f, 1.0f);
    }
    drawQuad(x, y, slotSize, slotSize);

    if (item)
    {
        auto itemData = ITEMS[(*item).itemID];
        int tileX = itemData.tileCoord.x;
        int tileY = itemData.tileCoord.y;

        if (tileX >= 0 && tileY >= 0)
        {
            glEnable(GL_TEXTURE_2D);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            drawAtlasQuad(x + 7.0f, y + 17.0f, slotSize - 24.0f, slotSize - 24.0f, tileX, tileY, 16);

            std::string text = std::to_string((*item).quantity);
            textRenderer.RenderText(text, x + slotSize - 10.0f, y + 10.0f, 0.7f, glm::vec3(0.0f, 0.0f, 0.0f), TextAlign::End);
        }
    }
}

void DrawHotbar(std::optional<Item> hotbar[8], float endXHotbar, float slotSize, float padding, float y, TextRenderer &textRenderer, int slotPressed)
{
    float startXHotbar = endXHotbar - 8 * (slotSize + padding) - padding;
    glColor4f(0.25f, 0.25f, 0.25f, 1.0f);
    drawQuad(startXHotbar - padding, y, 8 * (slotSize + padding) + padding, slotSize + 2 * padding);
    y = y + padding;
    for (int i = 0; i < 8; ++i)
    {
        float x = startXHotbar + i * (slotSize + padding);
        DrawItemWindow(hotbar[i], x, y, slotSize, textRenderer, slotPressed == i + 24);
    }

    // glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    // drawQuad(startXHotbar - padding / 2, y + padding + slotSize, 8 * (slotSize + padding), 4.0f);
}

void DrawInventorySlots(float endX, float slotSize, float padding, float yStart, std::optional<Item> inventory[24], TextRenderer &textRenderer, int slotPressed)
{
    float startX = endX - 8 * (slotSize + padding) - padding;
    glColor4f(0.25f, 0.25f, 0.25f, 1.0f);
    drawQuad(startX - padding, yStart, 8 * (slotSize + padding) + padding, 3 * (slotSize + padding) + padding);

    for (int z = 0; z < 3; ++z)
    {
        float y = yStart + z * (slotSize + padding) + padding;
        for (int i = 0; i < 8; ++i)
        {
            int slotNum = (2 - z) * 8 + i;
            float x = startX + i * (slotSize + padding);
            DrawItemWindow(inventory[slotNum], x, y, slotSize, textRenderer, slotPressed == slotNum);
        }
    }
}

void DrawCrafting(float startX, float slotSize, float padding, float yStart, TextRenderer &textRenderer, int nOfSlots, int slotPressed, std::optional<Item> crafting[4], std::optional<Item> crafted)
{
    glColor4f(0.25f, 0.25f, 0.25f, 1.0f);
    drawQuad(startX, yStart, 3 * slotSize + 4 * padding, 6.5f * padding + 4 * slotSize);

    float x = startX + padding + (slotSize + padding) / 2 * (3 - nOfSlots);
    for (int i = 0; i < nOfSlots; ++i)
    {
        float y = yStart + slotSize + padding * 3.5f + (slotSize + padding) * (2 - i) - (slotSize + padding) / 2 * (3 - nOfSlots);
        for (int j = 0; j < nOfSlots; ++j)
        {
            int slotNum = 32 + i * 2 + j;
            float xPos = x + j * (slotSize + padding);
            DrawItemWindow(crafting[i * 2 + j], xPos, y, slotSize, textRenderer, slotPressed == slotNum);
        }
    }

    std::optional<Item> pushItem = crafted;
    if (!pushItem)
    {
        pushItem = CalculateItem(crafting);
    }

    if (crafted && slotPressed != 36)
    {
        DrawItemWindow(pushItem, startX + (padding + slotSize) + padding, yStart + padding, slotSize, textRenderer, true, {1.0f, 0.84f, 0.45f});
    }
    else
    {
        DrawItemWindow(pushItem, startX + (padding + slotSize) + padding, yStart + padding, slotSize, textRenderer, slotPressed == 36);
    }
}

void DrawInventory(int screenWidth, int screenHeight, std::optional<Item> hotbar[8], std::optional<Item> inventory[24], std::optional<Item> crafting[4], std::optional<Item> crafted, TextRenderer &textRenderer, int slotPressed, float slotSize, float padding)
{
    glUseProgram(0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    float windowHeight = 8.5f * padding + 4 * slotSize;
    float windowWidth = 8 * slotSize + 11 * padding + (3 * slotSize + 4.5f * padding);

    float startX = (screenWidth - windowWidth) / 2;
    float y = (screenHeight - windowHeight) / 2;
    float endX = startX + windowWidth;

    // DRAW WINDOW BACKGROUND
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
    drawQuad(startX, y, windowWidth, windowHeight);

    // DRAW HOTBAR + DIVIDER
    DrawHotbar(hotbar, endX, slotSize, padding, y + padding, textRenderer, slotPressed);

    // DRAW INVENTORY SLOTS
    DrawInventorySlots(endX, slotSize, padding, y + 3.5f * padding + slotSize, inventory, textRenderer, slotPressed);

    // DRAW CRAFTING
    DrawCrafting(startX + padding, slotSize, padding, y + padding, textRenderer, 2, slotPressed, crafting, crafted);
}

int GetInventorySlotFromMouse(double mouseX, double mouseY, int screenWidth, int screenHeight, float slotSize, float padding)
{
    float windowHeight = 8.5f * padding + 4 * slotSize;
    float windowWidth = 8 * slotSize + 11 * padding + (3 * slotSize + 5 * padding);

    float startX = (screenWidth - windowWidth) / 2.0f;
    float startY = (screenHeight - windowHeight) / 2.0f;
    float endX = startX + windowWidth;

    // Inventory slots: slots 0–23
    for (int row = 0; row < 3; ++row)
    {
        float y = startY + row * (slotSize + padding) + 4.5f * padding + slotSize;
        for (int col = 0; col < 8; ++col)
        {
            float x = endX - 8 * (slotSize + padding) - padding + col * (slotSize + padding);

            if (mouseX >= x && mouseX <= x + slotSize &&
                mouseY >= y && mouseY <= y + slotSize)
            {
                int slotIndex = (2 - row) * 8 + col; // Inventory slot 0–23
                return slotIndex;
            }
        }
    }

    // Hotbar slots: slots 24–31
    float hotbarY = startY + padding + padding;
    float hotbarX = endX - 8 * (slotSize + padding) - padding;

    for (int col = 0; col < 8; ++col)
    {
        float x = hotbarX + col * (slotSize + padding);
        if (mouseX >= x && mouseX <= x + slotSize &&
            mouseY >= hotbarY && mouseY <= hotbarY + slotSize)
        {
            return 24 + col;
        }
    }

    int craftingSlots = 2;
    float x = startX + 2 * padding + (slotSize + padding) / 2 * (3 - craftingSlots);
    for (int i = 0; i < craftingSlots; ++i)
    {
        float y = startY + slotSize + padding * 4.5f + (slotSize + padding) * (2 - i) - (slotSize + padding) / 2 * (3 - craftingSlots);
        for (int j = 0; j < craftingSlots; ++j)
        {
            float xPos = x + j * (slotSize + padding);
            if (mouseX >= xPos && xPos + slotSize > mouseX && mouseY >= y && mouseY < y + slotSize)
            {
                return 32 + i * 2 + j;
            }
        }
    }

    // startX + (padding + slotSize) + padding, yStart + padding
    // DrawCrafting(startX + padding, slotSize, padding, y + padding

    // x = startX + (padding + slotSize) + padding * 2
    // y = hotbarY

    float x2 = startX + (padding + slotSize) + 2 * padding;
    if (mouseX >= x2 && x2 + slotSize > mouseX && mouseY >= hotbarY && mouseY < hotbarY + slotSize)
    {
        return 36;
    }

    return -1; // No slot clicked
}
