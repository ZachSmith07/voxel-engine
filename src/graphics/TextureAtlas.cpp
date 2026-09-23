#include "TextureAtlas.h"

std::array<glm::vec2, 4> TextureAtlas::GetTileUVs(int tileX, int tileY) {
    float u = tileX / tileCount;
    float v = tileY / tileCount;
    float tileW = 1.0f / tileCount;

    return {
        glm::vec2(u, v),                 // bottom-left
        glm::vec2(u + tileW, v),         // bottom-right
        glm::vec2(u + tileW, v + tileW), // top-right
        glm::vec2(u, v + tileW)          // top-left
    };
}
