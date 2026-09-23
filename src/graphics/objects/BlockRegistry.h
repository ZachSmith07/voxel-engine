#pragma once

#include <array>
#include <glm/glm.hpp>

// Face order: +X, -X, +Y, -Y, +Z, -Z
struct BlockFaceUVs {
    std::array<glm::ivec2, 6> faceUVs;
};

class BlockRegistry {
public:
    static void Init(); // call once at startup
    static BlockFaceUVs GetBlockUVs(int blockID);

private:
    static std::array<BlockFaceUVs, 256> blockData;
};
