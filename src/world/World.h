#pragma once

#include <optional>
#include "Chunk.h"
#include "ChunkCoord.h"
#include "ChunkManager.h"
#include <unordered_map>
#include <glm/glm.hpp>
#include <unordered_set>
#include <queue>
#include "DroppedItem.h"
#include "../interactable/CraftingTable.h"

class Shader;
class Camera;
class Chunk;


struct Vec3Hash {
    std::size_t operator()(const glm::vec3& v) const {
        std::size_t h1 = std::hash<float>{}(v.x);
        std::size_t h2 = std::hash<float>{}(v.y);
        std::size_t h3 = std::hash<float>{}(v.z);

        
        std::size_t seed = 0;
        seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};


class World
{
public:
    void Update(const Camera& camera);
    void Draw(Shader& shader) const;
    std::optional<glm::vec3> GetTargetBlock(const glm::vec3& origin, const glm::vec3& direction, const bool& place=false);
    void SetBlock(const int& x, const int& y, const int& z, int blockID, bool isRemoving=true);
    void DrawDroppedItems(Shader& shader, Camera camera);
    void MergeDroppedItems(Camera camera);
    int GetBlock(const int& x, const int& y, const int& z);
    std::vector<DroppedItem> pickupItems(Camera& camera);
    std::unordered_map<glm::vec3, CraftingTable, Vec3Hash> craftingTableLocations;

private:
    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> chunks;
    ChunkCoord GetChunk(const int& x, const int& y, const int& z);
    std::queue<ChunkCoord> chunkBuildQueue;
    std::unordered_set<ChunkCoord> scheduledChunks;
    std::unique_ptr<Chunk> GenerateNewChunk(const int& x, const int& z);
    std::unordered_map<ChunkCoord, std::future<std::unique_ptr<Chunk>>> chunkFutures;

    Chunk* GetChunkAtWorld(int worldX, int worldZ);
    bool SetBlockLight(int worldX, int worldY, int worldZ, uint8_t light);
    uint8_t GetBlockLight(int worldX, int worldY, int worldZ);

    std::vector<DroppedItem> droppedItems;

    void CheckSpecialPlace(const int &x, const int &y, const int &z, int blockID);

    
};
