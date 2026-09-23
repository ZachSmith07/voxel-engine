#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Hotbar.h"
#include "Basics.h"
#include "../player/ItemDatabase.h"

extern unsigned int textureID;

void DrawHotbar(int screenWidth, int screenHeight, std::optional<Item> items[8], TextRenderer &textRenderer, int hotbarIndex, int numSlots, float slotSize, float padding)
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

    float totalWidth = numSlots * slotSize + (numSlots - 1) * padding;
    float startX = (screenWidth - totalWidth) / 2.0f;
    float y = 20.0f;

    for (int i = 0; i < numSlots; ++i)
    {
        float x = startX + i * (slotSize + padding);

        glDisable(GL_TEXTURE_2D);
        if (hotbarIndex == i)
        {
            glColor4f(0.45f, 0.45f, 0.45f, 0.85f);
        }
        else
        {
            glColor4f(0.7f, 0.7f, 0.7f, 0.85f);
        }
        drawQuad(x, y, slotSize, slotSize);

        if (items[i])
        {
            auto itemData = ITEMS[(*items[i]).itemID];
            int tileX = itemData.tileCoord.x;
            int tileY = itemData.tileCoord.y;

            if (tileX >= 0 && tileY >= 0)
            {
                glEnable(GL_TEXTURE_2D);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                drawAtlasQuad(x + 7.0f, y + 17.0f, slotSize - 24.0f, slotSize - 24.0f, tileX, tileY, 16);

                std::string text = std::to_string((*items[i]).quantity);
                textRenderer.RenderText(text, x + slotSize - 10.0f, y + 10.0f, 0.7f, glm::vec3(0.0f, 0.0f, 0.0f), TextAlign::End);
            }
        }
    }

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
}