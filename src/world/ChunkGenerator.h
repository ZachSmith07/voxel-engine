// ChunkGenerator.h
#pragma once

#include <array>
#include "Chunk.h" // defines CHUNK_SIZE, CHUNK_HEIGHT
#include "ChunkCoord.h"

using BlockArray = std::array<std::array<std::array<int, CHUNK_SIZE>, CHUNK_HEIGHT>, CHUNK_SIZE>;

void GenerateChunk(BlockArray &blocks, int posX, int posZ);
// void GenerateChunkUnconcurrent(BlockArray &blocks, int posX, int posZ);
