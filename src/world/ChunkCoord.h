#pragma once
#include <functional>

struct ChunkCoord {
    int x, z;

    ChunkCoord(int x_, int z_) : x(x_), z(z_) {}

    bool operator==(const ChunkCoord& other) const {
        return x == other.x && z == other.z;
    }
};

namespace std {
    template <>
    struct hash<ChunkCoord> {
        size_t operator()(const ChunkCoord& c) const {
            return std::hash<int>()(c.x) ^ (std::hash<int>()(c.z) << 1);
        }
    };
}


struct ExtraBlock
{
    int x; // localX
    int y; // localY
    int z; // localZ
    int blockID;
};