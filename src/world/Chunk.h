#pragma once

#include <array>
#include <vector>
#include <atomic>
#include <memory>
#include <cstdint>
#include <glm/glm.hpp>
#include "../graphics/Shader.h"
#include "ChunkCoord.h"
#include <random>

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 256;

class Chunk
{
public:
    Chunk(int x, int z, int seed);

    ChunkCoord chunkCoord;

    void Generate();
    void BuildMeshData();
    void UploadMesh();
    void Draw(Shader &shader) const;

    int GetBlock(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, int blockID);
    // void CarveWormCave(glm::vec3 start, glm::vec3 direction, float length, float radius);
    void CarveWorm(glm::vec3 pos, glm::vec3 dir, std::mt19937 &rng, int depth, int maxDepth, float stepSize, float radius, float chunkWorldX, float chunkWorldZ, std::array<std::array<std::array<int, CHUNK_SIZE>, CHUNK_HEIGHT>, CHUNK_SIZE> &blocks);

    int posX, posZ, seed;
    std::atomic<bool> isGenerated = false;

    std::array<std::array<std::array<uint8_t, CHUNK_SIZE>, CHUNK_HEIGHT>, CHUNK_SIZE> blockLight;
    std::array<std::array<std::array<uint8_t, CHUNK_SIZE>, CHUNK_HEIGHT>, CHUNK_SIZE> skyLight;
    std::array<std::array<std::array<int, CHUNK_SIZE>, CHUNK_HEIGHT>, CHUNK_SIZE> blocks;
private:

    std::vector<float> vertexData;
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    int vertexCount = 0;

    bool IsBlockVisible(int x, int y, int z) const;
};
