#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Basics.h"

extern unsigned int textureID;

void drawQuad(float x, float y, float width, float height)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawAtlasQuad(float x, float y, float width, float height, int tileX, int tileY, int tilesPerRow)
{
    glBindTexture(GL_TEXTURE_2D, textureID);

    float tileSizeUV = 1.0f / tilesPerRow;

    float uMin = tileX * tileSizeUV;
    float vMax = 1.0f - tileY * tileSizeUV;       // flipped vertically
    float uMax = uMin + tileSizeUV;
    float vMin = vMax - tileSizeUV;

    glBegin(GL_QUADS);
        glTexCoord2f(uMin, vMin); glVertex2f(x, y);
        glTexCoord2f(uMax, vMin); glVertex2f(x + width, y);
        glTexCoord2f(uMax, vMax); glVertex2f(x + width, y + height);
        glTexCoord2f(uMin, vMax); glVertex2f(x, y + height);
    glEnd();
}