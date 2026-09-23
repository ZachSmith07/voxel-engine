#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <queue>
#include "Chunk.h"
#include "ChunkCoord.h"
#include "ThreadPool.h"

class ChunkManager {
public:
    ChunkManager();

    // true if the chunk was successfully SCHEDULED
    bool RequestChunk(int x, int z, int seed);

    // pulls finished chunks and upload them on the main thread
    void UpdateMainThread(std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>>& worldChunks,
                          std::unordered_set<ChunkCoord>& scheduledChunks);

private:
    ThreadPool threadPool;
    std::mutex readyMutex;

    std::queue<std::pair<ChunkCoord, std::shared_ptr<Chunk>>> readyChunks;

    std::atomic<int> activeChunkTasks = 0;
    const int maxConcurrentChunkTasks = 1;
};
