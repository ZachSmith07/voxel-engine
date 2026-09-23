#include "BlockRegistry.h"

// Each blockID maps to 6 faces (tileX, tileY)
std::array<BlockFaceUVs, 256> BlockRegistry::blockData;

void BlockRegistry::Init() {
    // Block 0 = dirt, all faces = (2, 0)
    blockData[0] = {{
        glm::ivec2(2, 0), // +X
        glm::ivec2(2, 0), // -X
        glm::ivec2(2, 0), // +Y
        glm::ivec2(2, 0), // -Y
        glm::ivec2(2, 0), // +Z
        glm::ivec2(2, 0)  // -Z
    }};

    // Example: Block 1 = grass (top green, bottom dirt, sides = grass)
    blockData[1] = {{
        glm::ivec2(3, 0), // +X (side)
        glm::ivec2(3, 0), // -X
        glm::ivec2(2, 0), // +Y (top - grass)
        glm::ivec2(2, 0), // -Y (bottom - dirt)
        glm::ivec2(3, 0), // +Z
        glm::ivec2(3, 0)  // -Z
    }};
}

BlockFaceUVs BlockRegistry::GetBlockUVs(int blockID) {
    return blockData[blockID];
}
