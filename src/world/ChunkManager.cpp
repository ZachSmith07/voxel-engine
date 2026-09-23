#include "ChunkManager.h"
#include <iostream>
#include <chrono>

ChunkManager::ChunkManager()
    : threadPool(std::max(8u, std::thread::hardware_concurrency())) {}

bool ChunkManager::RequestChunk(int x, int z, int seed) {
    if (activeChunkTasks >= maxConcurrentChunkTasks)
        return false;

    ChunkCoord coord{x, z};
    activeChunkTasks++;

    threadPool.enqueue([this, coord, x, z, seed]() {
        auto chunk = std::make_shared<Chunk>(x, z, seed);
        chunk->Generate();
        chunk->BuildMeshData();

        {
            std::lock_guard<std::mutex> lock(readyMutex);
            readyChunks.emplace(coord, chunk);
        }

        activeChunkTasks--;
    });

    return true;
}

void ChunkManager::UpdateMainThread(std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>>& worldChunks,
                                    std::unordered_set<ChunkCoord>& scheduledChunks)
{
    std::lock_guard<std::mutex> lock(readyMutex);

    while (!readyChunks.empty()) {
        auto [coord, chunk] = readyChunks.front();
        readyChunks.pop();

        

        if (!chunk) continue;

        std::cout << "Thing: "
                  << " ms (" << coord.x << "," << coord.z << ")\n";

        chunk->UploadMesh();
        worldChunks[coord] = chunk;

        scheduledChunks.erase(coord);
    }
}
