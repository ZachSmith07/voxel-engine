#include "World.h"
#include "Chunk.h"
#include "../graphics/Camera.h"
#include <glm/glm.hpp>
#include "../graphics/Shader.h"
#include <cmath>
#include <chrono>
#include <cmath>
#include <unordered_set>
#include <iostream>
#include <vector>
#include <unordered_set>
#include "../graphics/objects/BlockDatabase.h"

static inline int floorDiv(int v, int d) { return (v >= 0) ? v / d : ((v + 1) / d) - 1; }
static inline int posMod(int v, int d)
{
    int m = v % d;
    return (m < 0) ? m + d : m;
}

void World::Update(const Camera &camera)
{
    int playerChunkX = static_cast<int>(std::floor(camera.Position.x / 16.0f));
    int playerChunkZ = static_cast<int>(std::floor(camera.Position.z / 16.0f));
    const int renderDistance = 16;
    const int maxActiveFutures = 4;

    // STEP 1: Unload far chunks
    std::vector<ChunkCoord> toRemove;
    for (const auto &pair : chunks)
    {
        int dx = pair.first.x - playerChunkX;
        int dz = pair.first.z - playerChunkZ;
        if (std::abs(dx) > renderDistance || std::abs(dz) > renderDistance)
        {
            toRemove.push_back(pair.first);
        }
    }
    for (const auto &coord : toRemove)
    {
        chunks.erase(coord);
        scheduledChunks.erase(coord);
    }

    // STEP 2: Finalize any completed chunk generation
    std::vector<ChunkCoord> completed;
    for (auto it = chunkFutures.begin(); it != chunkFutures.end();)
    {
        auto &future = it->second;
        if (future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
        {
            std::unique_ptr<Chunk> chunk = future.get();
            chunk->UploadMesh();
            chunks[chunk->chunkCoord] = std::move(chunk);
            completed.push_back(it->first);
            it = chunkFutures.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (const auto &coord : completed)
    {
        scheduledChunks.erase(coord);
    }

    // STEP 3: If we have room, find the closest new chunks to build
    int numNeeded = maxActiveFutures - static_cast<int>(chunkFutures.size());
    if (numNeeded > 0)
    {
        std::vector<std::pair<ChunkCoord, int>> candidates;

        for (int dx = -renderDistance; dx <= renderDistance; ++dx)
        {
            for (int dz = -renderDistance; dz <= renderDistance; ++dz)
            {
                ChunkCoord coord{playerChunkX + dx, playerChunkZ + dz};

                if (chunks.find(coord) == chunks.end() &&
                    scheduledChunks.find(coord) == scheduledChunks.end() &&
                    chunkFutures.find(coord) == chunkFutures.end())
                {
                    int distSq = dx * dx + dz * dz;
                    candidates.emplace_back(coord, distSq);
                }
            }
        }

        std::sort(candidates.begin(), candidates.end(),
                  [](const auto &a, const auto &b)
                  {
                      return a.second < b.second;
                  });

        for (int i = 0; i < std::min(numNeeded, static_cast<int>(candidates.size())); ++i)
        {
            ChunkCoord coord = candidates[i].first;
            scheduledChunks.insert(coord);

            chunkFutures[coord] = std::async(std::launch::async, [this, coord]()
                                             { return this->GenerateNewChunk(coord.x, coord.z); });
        }
    }
}

std::unique_ptr<Chunk> World::GenerateNewChunk(const int &x, const int &z)
{
    auto start = std::chrono::high_resolution_clock::now();

    auto chunk = std::make_unique<Chunk>(x, z, 0);

    auto genStart = std::chrono::high_resolution_clock::now();
    chunk->Generate();
    auto genEnd = std::chrono::high_resolution_clock::now();

    auto meshStart = std::chrono::high_resolution_clock::now();
    chunk->BuildMeshData();
    auto meshEnd = std::chrono::high_resolution_clock::now();

    auto end = std::chrono::high_resolution_clock::now();

    double genTime = std::chrono::duration<double, std::milli>(genEnd - genStart).count();
    double meshTime = std::chrono::duration<double, std::milli>(meshEnd - meshStart).count();
    double totalTime = std::chrono::duration<double, std::milli>(end - start).count();

    // std::cout << "Generate() took " << genTime << " ms\n";
    // std::cout << "BuildMeshData() took " << meshTime << " ms\n";
    // std::cout << "Total GenerateNewChunk() took " << totalTime << " ms\n";

    return chunk;
}

void World::Draw(Shader &shader) const
{
    for (const auto &pair : chunks)
    {
        pair.second->Draw(shader);
    }
}

std::optional<glm::vec3> World::GetTargetBlock(const glm::vec3 &origin, const glm::vec3 &direction, const bool &place)
{
    constexpr int MAX_STEPS = 16;

    glm::vec3 dir = glm::normalize(direction);

    int x = static_cast<int>(std::floor(origin.x));
    int y = static_cast<int>(std::floor(origin.y));
    int z = static_cast<int>(std::floor(origin.z));

    glm::vec3 deltaDist = {
        dir.x != 0.0f ? std::abs(1.0f / dir.x) : 1e30f,
        dir.y != 0.0f ? std::abs(1.0f / dir.y) : 1e30f,
        dir.z != 0.0f ? std::abs(1.0f / dir.z) : 1e30f};

    glm::ivec3 step = {
        (dir.x < 0) ? -1 : 1,
        (dir.y < 0) ? -1 : 1,
        (dir.z < 0) ? -1 : 1};

    glm::vec3 sideDist = {
        (dir.x < 0) ? (origin.x - x) * deltaDist.x : (x + 1.0f - origin.x) * deltaDist.x,
        (dir.y < 0) ? (origin.y - y) * deltaDist.y : (y + 1.0f - origin.y) * deltaDist.y,
        (dir.z < 0) ? (origin.z - z) * deltaDist.z : (z + 1.0f - origin.z) * deltaDist.z};

    int prevX = x;
    int prevY = y;
    int prevZ = z;

    for (int i = 0; i < MAX_STEPS; ++i)
    {
        int block = GetBlock(x, y, z);

        if (block != 0)
        {
            if (place)
            {
                // Return the empty block before this solid block
                return glm::vec3(prevX, prevY, prevZ);
            }
            else
            {
                // Return the hit block itself
                return glm::vec3(x, y, z);
            }
        }

        // Store previous empty block coords
        prevX = x;
        prevY = y;
        prevZ = z;

        // DDA step
        if (sideDist.x < sideDist.y && sideDist.x < sideDist.z)
        {
            sideDist.x += deltaDist.x;
            x += step.x;
        }
        else if (sideDist.y < sideDist.z)
        {
            sideDist.y += deltaDist.y;
            y += step.y;
        }
        else
        {
            sideDist.z += deltaDist.z;
            z += step.z;
        }
    }

    return std::nullopt; // no block hit
}

ChunkCoord World::GetChunk(const int &x, const int & /*y*/, const int &z)
{
    return {floorDiv(x, CHUNK_SIZE), floorDiv(z, CHUNK_SIZE)};
}

void World::DrawDroppedItems(Shader &shader, Camera camera)
{
    using namespace std::chrono;

    auto startUpdate = high_resolution_clock::now();

    // Update phase
    for (DroppedItem &item : droppedItems)
    {
        if (glm::distance(item.position, camera.Position) < 100.0f)
        {
            item.Update(*this);
        }
    }

    auto endUpdate = high_resolution_clock::now();
    auto updateDuration = duration_cast<microseconds>(endUpdate - startUpdate).count();

    auto startDraw = high_resolution_clock::now();

    // Draw phase
    for (DroppedItem &item : droppedItems)
    {
        if (glm::distance(item.position, camera.Position) < 250.0f)
        {
            item.Draw(shader);
        }
    }

    auto endDraw = high_resolution_clock::now();
    auto drawDuration = duration_cast<microseconds>(endDraw - startDraw).count();
}

void World::MergeDroppedItems(Camera camera)
{
    const float mergeDistance = 1.7f;

    std::vector<bool> merged(droppedItems.size(), false);

    for (size_t i = 0; i < droppedItems.size(); ++i)
    {
        if (merged[i]) continue;  // Skip if already merged

        for (size_t j = i + 1; j < droppedItems.size(); ++j)
        {
            if (merged[j]) continue;  // Skip if already merged

            if (droppedItems[i].itemID == droppedItems[j].itemID &&
                glm::distance(droppedItems[i].position, droppedItems[j].position) < mergeDistance)
            {
                // Example merge logic: Move items together (or stack them if you have a count)
                // Here you could also adjust position or other properties if needed

                // Mark j as merged (we'll delete later)
                merged[j] = true;
            }
        }
    }

    // Remove merged items in reverse to avoid index shifting
    for (int i = static_cast<int>(droppedItems.size()) - 1; i >= 0; --i)
    {
        if (merged[i])
            droppedItems.erase(droppedItems.begin() + i);
    }
}


int World::GetBlock(const int &x, const int &y, const int &z)
{
    const ChunkCoord coord = GetChunk(x, y, z);
    auto it = chunks.find(coord);
    if (it == chunks.end())
        return 0; // air if chunk not loaded

    const Chunk &chunk = *it->second;

    const int localX = posMod(x, CHUNK_SIZE);
    const int localZ = posMod(z, CHUNK_SIZE);
    const int localY = y; // only 0–15 exist in this demo

    return chunk.GetBlock(localX, localY, localZ);
}

std::vector<DroppedItem> World::pickupItems(Camera &camera)
{
    std::vector<DroppedItem> items;

    glm::vec3 camPosition = camera.Position - glm::vec3(0.0f, 1.4f, 0.0f);

    for (size_t i = 0; i < droppedItems.size(); )
    {
        if (glm::distance(droppedItems[i].position, camPosition) < 2.0f)
        {
            items.push_back(droppedItems[i]);

            // Remove this item safely
            droppedItems.erase(droppedItems.begin() + i);
            // Do NOT increment i because the next item shifted into position i
        }
        else
        {
            ++i; // Only increment if not erasing
        }
    }

    return items;
}

void World::CheckSpecialPlace(const int &x, const int &y, const int &z, int blockID)
{
    if (blockID == 15) // CRAFTING TABLE
    {

    }
}

void World::SetBlock(const int &x, const int &y, const int &z, int blockID, bool isRemoving)
{
    const ChunkCoord coord = GetChunk(x, y, z);
    auto it = chunks.find(coord);
    if (it == chunks.end())
        return;

    Chunk &chunk = *it->second;

    const int localX = posMod(x, CHUNK_SIZE);
    const int localZ = posMod(z, CHUNK_SIZE);
    const int localY = y;

    if (blockID == 0 && isRemoving)
    {
        DroppedItem droppedItem = DroppedItem(glm::vec3(x, y, z), BLOCKS[chunk.GetBlock(localX, localY, localZ)].itemNum, 1);
        std::cout << droppedItem << std::endl;
        droppedItems.push_back(droppedItem);
    }
    CheckSpecialPlace(x, y, z, blockID);

    chunk.SetBlock(localX, localY, localZ, blockID);
    chunk.BuildMeshData();
    chunk.UploadMesh();
}
