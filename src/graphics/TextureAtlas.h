#pragma once
#include <glm/glm.hpp>
#include <array>

class TextureAtlas {
public:
    static constexpr int atlasSize = 256;
    static constexpr int tileSize = 16;
    static constexpr float tileCount = atlasSize / tileSize;

    // Returns normalized UVs for one tile
    static std::array<glm::vec2, 4> GetTileUVs(int tileX, int tileY);
};
