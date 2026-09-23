#include "Chunk.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "../graphics/objects/BlockDatabase.h"
#include "../graphics/Shader.h"
#include "CubeUV.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "FastNoiseLite.h"
#include <chrono>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "ChunkGenerator.h"


uint32_t Hash(int x, int y, int z, int seed)
{
    uint32_t h = seed;
    h ^= x * 374761393u;
    h ^= y * 668265263u;
    h ^= z * 73856093u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h;
}

extern unsigned int atlasTexture;

Chunk::Chunk(int x, int z, int seed)
    : posX(x), posZ(z), seed(seed), isGenerated(false), chunkCoord(x, z)
{
    
}
void Chunk::CarveWorm(glm::vec3 pos, glm::vec3 dir, std::mt19937 &rng, int depth, int maxDepth, float stepSize, float radius, float chunkWorldX, float chunkWorldZ, std::array<std::array<std::array<int, CHUNK_SIZE>, CHUNK_HEIGHT>, CHUNK_SIZE> &blocks)
{
    std::uniform_real_distribution<float> turn(-0.15f, 0.15f);
    std::uniform_real_distribution<float> splitChance(0.0f, 1.0f);

    for (int i = 0; i < 100; ++i)
    {
        // Turn slightly
        dir += glm::vec3(turn(rng), turn(rng) * 0.5f, turn(rng));
        dir = glm::normalize(dir);

        pos += dir * stepSize;

        int xi = static_cast<int>(pos.x - chunkWorldX);
        int yi = static_cast<int>(pos.y);
        int zi = static_cast<int>(pos.z - chunkWorldZ);

        if (xi >= -radius && xi < CHUNK_SIZE + radius &&
            yi >= 1 && yi < CHUNK_HEIGHT - 1 &&
            zi >= -radius && zi < CHUNK_SIZE + radius)
        {
            // carve sphere
            for (int dx = -radius; dx <= radius; ++dx)
                for (int dy = -radius; dy <= radius; ++dy)
                    for (int dz = -radius; dz <= radius; ++dz)
                    {
                        glm::vec3 p = pos + glm::vec3(dx, dy, dz);
                        int lx = static_cast<int>(p.x - chunkWorldX);
                        int ly = static_cast<int>(p.y);
                        int lz = static_cast<int>(p.z - chunkWorldZ);

                        if (lx >= 0 && lx < CHUNK_SIZE &&
                            ly >= 0 && ly < CHUNK_HEIGHT &&
                            lz >= 0 && lz < CHUNK_SIZE)
                        {
                            if (glm::length2(glm::vec3(dx, dy, dz)) <= radius * radius)
                                blocks[lx][ly][lz] = 3; // Stone
                        }
                    }
        }

        // random chance to branch
        if (depth < maxDepth && splitChance(rng) < 0.02f)
        {
            glm::vec3 newDir = glm::normalize(glm::vec3(turn(rng), turn(rng) * 0.5f, turn(rng)));
            CarveWorm(pos, newDir, rng, depth + 1, maxDepth, stepSize, radius * 0.9f, chunkWorldX, chunkWorldZ, blocks);
        }
    }
}

// void Chunk::Generate()
// {
//     const int SEA_LEVEL = 100;

//     FastNoiseLite baseNoise, detailNoise, continentNoise;
//     baseNoise.SetSeed(seed);
//     baseNoise.SetFrequency(0.005f);
//     baseNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

//     detailNoise.SetSeed(seed + 100);
//     detailNoise.SetFrequency(0.03f);
//     detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

//     continentNoise.SetSeed(seed + 200);
//     continentNoise.SetFrequency(0.0015f);
//     continentNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

//     // Generate base terrain using heightmap
//     for (int x = 0; x < CHUNK_SIZE; ++x)
//     {
//         for (int z = 0; z < CHUNK_SIZE; ++z)
//         {
//             float worldX = posX * CHUNK_SIZE + x;
//             float worldZ = posZ * CHUNK_SIZE + z;

//             float continentVal = continentNoise.GetNoise(worldX, worldZ);
//             float continentFactor = glm::smoothstep(0.1f, 0.5f, (continentVal + 1.0f) * 0.5f);

//             float base = baseNoise.GetNoise(worldX, worldZ);
//             float detail = detailNoise.GetNoise(worldX, worldZ);

//             float heightF = pow((base + 1.0f) * 0.5f, 1.2f) + detail * 0.1f;
//             heightF *= 64.0f;
//             heightF += 110.0f;
//             heightF *= continentFactor;
//             heightF -= 28.0f;
//             heightF = glm::min(heightF, static_cast<float>(CHUNK_HEIGHT - 1));

//             int height = static_cast<int>(heightF);

//             for (int y = 0; y < CHUNK_HEIGHT; ++y)
//             {
//                 blocks[x][y][z] = (y <= height) ? 3 : 0;
//             }

//             // Surface logic
//             for (int y = CHUNK_HEIGHT - 1; y >= 0; --y)
//             {
//                 if (blocks[x][y][z] == 3)
//                 {
//                     if (y < SEA_LEVEL + 2)
//                         blocks[x][y][z] = 4; // Sand
//                     else
//                         blocks[x][y][z] = 1; // Grass
//                     break;
//                 }
//             }

//             // Water
//             for (int y = 0; y <= SEA_LEVEL; ++y)
//             {
//                 if (blocks[x][y][z] == 0)
//                     blocks[x][y][z] = 5;
//             }
//         }
//     }

//     // Carve long worm cave
//     for (int i = 0; i < 3; ++i) // Multiple worms
//     {
//         glm::vec3 start(
//             posX * CHUNK_SIZE + rand() % CHUNK_SIZE,
//             40 + rand() % 30,
//             posZ * CHUNK_SIZE + rand() % CHUNK_SIZE);

//         glm::vec3 dir = glm::normalize(glm::vec3(
//             0.5f - (rand() % 100) / 100.0f,
//             0.1f - (rand() % 100) / 500.0f, // mostly horizontal
//             0.5f - (rand() % 100) / 100.0f));

//         float len = 40.0f + rand() % 30;
//         float radius = 2.5f + rand() % 2;

//         CarveWormCave(start, dir, len, radius);
//     }

//     isGenerated = true;
// }

// void Chunk::Generate()
// {
//     const int SEA_LEVEL = 100;
//     const float TURBULENCE_FREQ = 0.02f;
//     const int GRID_STEP = 4;
//     const int DENSITY_HEIGHT_STEP = 8;

//     // --- Noise setup ---
//     FastNoiseLite heightNoise;
//     heightNoise.SetSeed(seed);
//     heightNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     heightNoise.SetFrequency(0.003f);

//     FastNoiseLite mountainNoise;
//     mountainNoise.SetSeed(seed + 1);
//     mountainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     mountainNoise.SetFrequency(0.002f);

//     FastNoiseLite turbulenceNoise;
//     turbulenceNoise.SetSeed(seed + 2);
//     turbulenceNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     turbulenceNoise.SetFrequency(TURBULENCE_FREQ);

//     // Coarse grid size
//     const int gx = CHUNK_SIZE / GRID_STEP + 1;
//     const int gz = CHUNK_SIZE / GRID_STEP + 1;
//     const int gy = CHUNK_HEIGHT / DENSITY_HEIGHT_STEP + 1;

//     float densityGrid[gx][gy][gz];

//     // Step 1: Fill coarse density grid
//     for (int dx = 0; dx < gx; ++dx)
//     {
//         for (int dz = 0; dz < gz; ++dz)
//         {
//             int worldX = posX * CHUNK_SIZE + dx * GRID_STEP;
//             int worldZ = posZ * CHUNK_SIZE + dz * GRID_STEP;

//             float baseHeight = heightNoise.GetNoise((float)worldX, (float)worldZ);
//             float mountain = mountainNoise.GetNoise((float)worldX, (float)worldZ);
//             float heightF = ((baseHeight + 1.0f) * 0.5f) * 64.0f;
//             float mountainFactor = glm::smoothstep(0.2f, 0.6f, (mountain + 1.0f) * 0.5f);

//             float maxHeight = glm::mix(heightF, 160.0f, mountainFactor);

//             for (int dy = 0; dy < gy; ++dy)
//             {
//                 int y = dy * DENSITY_HEIGHT_STEP;

//                 float turb = turbulenceNoise.GetNoise((float)worldX, (float)y, (float)worldZ);
//                 float density = maxHeight - y + turb * 10.0f;
//                 density += glm::max(4.0f * (64.0f - y), 0.0f); // Density bias near surface

//                 densityGrid[dx][dy][dz] = density;
//             }
//         }
//     }

//     // Step 2: Interpolate and fill blocks
//     for (int x = 0; x < CHUNK_SIZE; ++x)
//     {
//         for (int z = 0; z < CHUNK_SIZE; ++z)
//         {
//             int worldX = posX * CHUNK_SIZE + x;
//             int worldZ = posZ * CHUNK_SIZE + z;

//             int cx = x / GRID_STEP;
//             int cz = z / GRID_STEP;
//             float fx = (x % GRID_STEP) / (float)GRID_STEP;
//             float fz = (z % GRID_STEP) / (float)GRID_STEP;

//             for (int y = 0; y < CHUNK_HEIGHT; ++y)
//             {
//                 int cy = y / DENSITY_HEIGHT_STEP;
//                 float fy = (y % DENSITY_HEIGHT_STEP) / (float)DENSITY_HEIGHT_STEP;

//                 // Get 8-corner densities for trilinear interpolation
//                 float d000 = densityGrid[cx    ][cy    ][cz    ];
//                 float d100 = densityGrid[cx + 1][cy    ][cz    ];
//                 float d010 = densityGrid[cx    ][cy + 1][cz    ];
//                 float d110 = densityGrid[cx + 1][cy + 1][cz    ];
//                 float d001 = densityGrid[cx    ][cy    ][cz + 1];
//                 float d101 = densityGrid[cx + 1][cy    ][cz + 1];
//                 float d011 = densityGrid[cx    ][cy + 1][cz + 1];
//                 float d111 = densityGrid[cx + 1][cy + 1][cz + 1];

//                 float d00 = glm::mix(d000, d100, fx);
//                 float d10 = glm::mix(d010, d110, fx);
//                 float d01 = glm::mix(d001, d101, fx);
//                 float d11 = glm::mix(d011, d111, fx);

//                 float d0 = glm::mix(d00, d10, fy);
//                 float d1 = glm::mix(d01, d11, fy);

//                 float density = glm::mix(d0, d1, fz);

//                 // Block assignment
//                 if (density < 0.0f)
//                 {
//                     if (y <= SEA_LEVEL)
//                         blocks[x][y][z] = 5; // Water
//                     else
//                         blocks[x][y][z] = 0; // Air
//                 }
//                 else
//                 {
//                     if (y == CHUNK_HEIGHT - 1 || blocks[x][y + 1][z] == 0)
//                     {
//                         blocks[x][y][z] = (y < SEA_LEVEL + 2) ? 4 : 1; // Sand or Grass
//                     }
//                     else if (y > 0 && blocks[x][y - 1][z] == 0)
//                     {
//                         blocks[x][y][z] = 2; // Dirt
//                     }
//                     else
//                     {
//                         blocks[x][y][z] = 3; // Stone
//                     }
//                 }
//             }
//         }
//     }

//     isGenerated = true;
// }

void Chunk::Generate()
{
    GenerateChunk(blocks, posX, posZ);

    isGenerated = true;
}

// void Chunk::Generate()
// {
//     const int SEA_LEVEL = 100;

//     // --- 2D Terrain Noise ---
//     FastNoiseLite continentNoise;
//     continentNoise.SetSeed(seed + 500);
//     continentNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     continentNoise.SetFrequency(0.0015f);

//     FastNoiseLite terrainNoise;
//     terrainNoise.SetSeed(seed);
//     terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     terrainNoise.SetFrequency(0.005f);

//     FastNoiseLite detailNoise;
//     detailNoise.SetSeed(seed + 100);
//     detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     detailNoise.SetFrequency(0.03f);

//     FastNoiseLite ridgeNoise;
//     ridgeNoise.SetSeed(seed + 200);
//     ridgeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     ridgeNoise.SetFrequency(0.002f);

//     FastNoiseLite flattenNoise;
//     flattenNoise.SetSeed(seed + 300);
//     flattenNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     flattenNoise.SetFrequency(0.0025f);

//     FastNoiseLite beachNoise;
//     beachNoise.SetSeed(seed + 321);
//     beachNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     beachNoise.SetFrequency(0.008f);

//     FastNoiseLite lakeNoise;
//     lakeNoise.SetSeed(seed + 654);
//     lakeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     lakeNoise.SetFrequency(0.01f);

//     FastNoiseLite riverNoise;
//     riverNoise.SetSeed(seed + 777);
//     riverNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     riverNoise.SetFrequency(0.0025f);

//     FastNoiseLite riverWarp;
//     riverWarp.SetSeed(seed + 888);
//     riverWarp.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
//     riverWarp.SetFrequency(0.002f);
//     riverWarp.SetFractalType(FastNoiseLite::FractalType_FBm);
//     riverWarp.SetFractalOctaves(3);

//     for (int x = 0; x < CHUNK_SIZE; ++x)
//     {
//         for (int z = 0; z < CHUNK_SIZE; ++z)
//         {
//             float worldX = posX * CHUNK_SIZE + x;
//             float worldZ = posZ * CHUNK_SIZE + z;

//             float continentVal = continentNoise.GetNoise(worldX, worldZ);
//             float continentFactor = glm::smoothstep(0.1f, 0.5f, (continentVal + 1.0f) * 0.5f);

//             float flatten = flattenNoise.GetNoise(worldX, worldZ);
//             float flattenFactor = glm::clamp((flatten + 1.0f) * 0.5f, 0.0f, 1.0f);
//             float landFlattenBias = glm::clamp(continentFactor * 1.5f + 0.2f, 0.0f, 1.0f);
//             float fullFlattenFactor = glm::smoothstep(0.0f, 1.0f, flattenFactor * landFlattenBias);

//             float base = terrainNoise.GetNoise(worldX, worldZ);
//             float detail = detailNoise.GetNoise(worldX, worldZ);
//             float ridge = fabs(ridgeNoise.GetNoise(worldX, worldZ));
//             float beachChance = (beachNoise.GetNoise(worldX, worldZ) + 1.0f) * 0.5f;

//             float baseHeight = pow((base + 1.0f) * 0.5f, 1.2f);
//             float shaped = glm::mix(baseHeight, 0.5f, fullFlattenFactor);

//             float ridgeInfluence = glm::mix(ridge * 4.0f, ridge * 10.0f, 1.0f - fullFlattenFactor);
//             float detailInfluence = glm::mix(detail * 1.0f, detail * 3.0f, 1.0f - fullFlattenFactor);

//             float shapedHeight = shaped * 64.0f;
//             float heightF = shapedHeight + ridgeInfluence + detailInfluence;

//             // Lake dip
//             float lakeDip = lakeNoise.GetNoise(worldX, worldZ);
//             float lakeFactor = glm::smoothstep(0.3f, 0.8f, (lakeDip + 1.0f) * 0.5f);
//             heightF -= lakeFactor * 4.0f;

//             // River carving
//             float angle = 0.6f;
//             float dx = cos(angle);
//             float dz = sin(angle);
//             float warpStrength = 100.0f;
//             float warpX = riverWarp.GetNoise(worldX, worldZ) * warpStrength;
//             float warpZ = riverWarp.GetNoise(worldX + 1000.0f, worldZ + 1000.0f) * warpStrength;
//             float warpedX = worldX + warpX;
//             float warpedZ = worldZ + warpZ;
//             float riverPos = warpedX * dx + warpedZ * dz;
//             float riverVal = riverNoise.GetNoise(riverPos, 0.0f);
//             float riverDist = fabs(riverVal);
//             float riverAmount = glm::smoothstep(0.2f, 0.0f, riverDist);
//             float softenedAmount = glm::mix(riverAmount, pow(riverAmount, 1.5f), 0.5f);
//             float continentMask = glm::smoothstep(0.2f, 0.5f, continentFactor);
//             float riverDepth = softenedAmount * 25.0f * continentMask;
//             heightF -= riverDepth;

//             // Final terrain lift
//             heightF += 110.0f;
//             heightF *= continentFactor;
//             heightF -= 28.0f;

//             heightF = glm::min(heightF, static_cast<float>(CHUNK_HEIGHT - 1));

//             // Seafloor softening
//             if (heightF < SEA_LEVEL)
//             {
//                 float diff = SEA_LEVEL - heightF;
//                 heightF = SEA_LEVEL - pow(diff, 0.7f);
//             }

//             int height = static_cast<int>(heightF);

//             for (int y = CHUNK_HEIGHT - 1; y >= 0; --y)
//             {
//                 if (y > height)
//                 {
//                     blocks[x][y][z] = 0; // Air
//                 }
//                 else
//                 {
//                     bool isSurface = (y == height);
//                     bool isCliff = (ridge > 0.6f && fullFlattenFactor < 0.4f);

//                     if (isSurface)
//                     {
//                         if (height < SEA_LEVEL + 2 && beachChance < 0.7f)
//                             blocks[x][y][z] = 4; // Sand
//                         else if (isCliff)
//                             blocks[x][y][z] = 3; // Stone
//                         else
//                             blocks[x][y][z] = 1; // Grass
//                     }
//                     else if (y > height - 4)
//                     {
//                         uint8_t surfaceBlock = blocks[x][height][z];
//                         if (surfaceBlock == 1)
//                             blocks[x][y][z] = 2; // Dirt
//                         else if (surfaceBlock == 4)
//                             blocks[x][y][z] = 4; // Sand
//                         else
//                         {
//                             int localRand = static_cast<int>((x * 73856093) + (y * 19349663) + (z * 83492791) + seed) & 0x7FFFFFFF;
//                             blocks[x][y][z] = (localRand % 10 == 0) ? 2 : 3;
//                         }
//                     }
//                     else
//                     {
//                         blocks[x][y][z] = 3; // Stone
//                     }
//                 }
//             }

//             // Water fill
//             for (int y = 0; y <= SEA_LEVEL; ++y)
//             {
//                 if (blocks[x][y][z] == 0)
//                     blocks[x][y][z] = 5; // Water
//             }
//         }
//     }

//     isGenerated = true;
// }

bool Chunk::IsBlockVisible(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_HEIGHT || // ✅ Correct vertical limit
        z < 0 || z >= CHUNK_SIZE)
        return true;

    return blocks[x][y][z] == 0;
}

void Chunk::BuildMeshData()
{
    auto start = std::chrono::high_resolution_clock::now();

    vertexData.clear();
    vertexData.reserve(150000); // Good reservation, helps performance

    const float tileSize = 1.0f / 16.0f;
    const float chunkOffsetX = posX * CHUNK_SIZE;
    const float chunkOffsetZ = posZ * CHUNK_SIZE;

    const float constantBrightness = 1.0f; // This is a placeholder, consider dynamic lighting later

    auto addQuad = [&](float x1, float y1, float z1, float x2, float y2, float z2,
                       int tileX, int tileY, bool flip, int faceDir) { // 'flip' parameter is unused
        float uMin = tileX * tileSize;
        float uMax = uMin + tileSize;
        float vMax = 1.0f - tileY * tileSize; // V=0 is top of atlas, V=1 is bottom
        float vMin = vMax - tileSize;         // So vMin is top edge of tile, vMax is bottom edge

        // Helper for the rotated UVs based on DroppedItem's successful pattern
        // This set assumes the default `addQuad` coordinates for each face
        // result in an "upright" texture, and we want to flip them 180 degrees.
        // It maps:
        // Vertex 0 -> Original TR (uMax, vMin)
        // Vertex 1 -> Original TL (uMin, vMin)
        // Vertex 2 -> Original BL (uMin, vMax)
        // Vertex 3 -> Original BR (uMax, vMax)
        // (uv sequence for two triangles: V0, V1, V2, V2, V3, V0)
        // So: TR, TL, BL, BL, BR, TR
        // This is the specific 180-degree rotation that was found to work for DroppedItem.

        if (faceDir == 0)
        { // -Z (Front Face)
            vertexData.insert(vertexData.end(), {
                                                    // (x1, y1, z1) -> BL of quad
                                                    // (x2, y1, z1) -> BR of quad
                                                    // (x2, y2, z1) -> TR of quad
                                                    // (x1, y2, z1) -> TL of quad
                                                    x1, y1, z1, uMax, vMin, constantBrightness, // Pos (x1,y1,z1) gets UV (uMax,vMin) -> Original TR
                                                    x2, y1, z1, uMin, vMin, constantBrightness, // Pos (x2,y1,z1) gets UV (uMin,vMin) -> Original TL
                                                    x2, y2, z1, uMin, vMax, constantBrightness, // Pos (x2,y2,z1) gets UV (uMin,vMax) -> Original BL

                                                    x2, y2, z1, uMin, vMax, constantBrightness, // Duplicate of above (Pos (x2,y2,z1) gets UV (uMin,vMax))
                                                    x1, y2, z1, uMax, vMax, constantBrightness, // Pos (x1,y2,z1) gets UV (uMax,vMax) -> Original BR
                                                    x1, y1, z1, uMax, vMin, constantBrightness  // Duplicate of first (Pos (x1,y1,z1) gets UV (uMax,vMin))
                                                });
        }
        else if (faceDir == 1)
        { // +Z (Back Face)
            // Same pattern as Front face for UV rotation
            vertexData.insert(vertexData.end(), {x1, y1, z2, uMax, vMin, constantBrightness,
                                                 x2, y1, z2, uMin, vMin, constantBrightness,
                                                 x2, y2, z2, uMin, vMax, constantBrightness,

                                                 x2, y2, z2, uMin, vMax, constantBrightness,
                                                 x1, y2, z2, uMax, vMax, constantBrightness,
                                                 x1, y1, z2, uMax, vMin, constantBrightness});
        }
        else if (faceDir == 2)
        { // -X (Left Face)
            // Same pattern as Front face for UV rotation
            vertexData.insert(vertexData.end(), {x1, y1, z1, uMax, vMin, constantBrightness,
                                                 x1, y1, z2, uMin, vMin, constantBrightness,
                                                 x1, y2, z2, uMin, vMax, constantBrightness,

                                                 x1, y2, z2, uMin, vMax, constantBrightness,
                                                 x1, y2, z1, uMax, vMax, constantBrightness,
                                                 x1, y1, z1, uMax, vMin, constantBrightness});
        }
        else if (faceDir == 3)
        { // +X (Right Face)
            // Same pattern as Front face for UV rotation
            vertexData.insert(vertexData.end(), {x2, y1, z1, uMax, vMin, constantBrightness,
                                                 x2, y1, z2, uMin, vMin, constantBrightness,
                                                 x2, y2, z2, uMin, vMax, constantBrightness,

                                                 x2, y2, z2, uMin, vMax, constantBrightness,
                                                 x2, y2, z1, uMax, vMax, constantBrightness,
                                                 x2, y1, z1, uMax, vMin, constantBrightness});
        }
        else if (faceDir == 4)
        { // -Y (Bottom Face)
            // Same pattern as Front face for UV rotation
            vertexData.insert(vertexData.end(), {x1, y1, z1, uMax, vMin, constantBrightness,
                                                 x2, y1, z1, uMin, vMin, constantBrightness,
                                                 x2, y1, z2, uMin, vMax, constantBrightness,

                                                 x2, y1, z2, uMin, vMax, constantBrightness,
                                                 x1, y1, z2, uMax, vMax, constantBrightness,
                                                 x1, y1, z1, uMax, vMin, constantBrightness});
        }
        else if (faceDir == 5)
        { // +Y (Top Face)
            // Same pattern as Front face for UV rotation
            vertexData.insert(vertexData.end(), {x1, y2, z1, uMax, vMin, constantBrightness,
                                                 x2, y2, z1, uMin, vMin, constantBrightness,
                                                 x2, y2, z2, uMin, vMax, constantBrightness,

                                                 x2, y2, z2, uMin, vMax, constantBrightness,
                                                 x1, y2, z2, uMax, vMax, constantBrightness,
                                                 x1, y2, z1, uMax, vMin, constantBrightness});
        }
    };

    for (int y = 0; y < CHUNK_HEIGHT; ++y)
    {
        for (int z = 0; z < CHUNK_SIZE; ++z)
        {
            for (int x = 0; x < CHUNK_SIZE; ++x)
            {
                int id = blocks[x][y][z];
                if (id == 0)
                    continue; // Skip air blocks

                float bx = chunkOffsetX + x;
                float by = static_cast<float>(y);
                float bz = chunkOffsetZ + z;

                // Retrieve tile coordinates for each face of the current block type
                int tile[6][2];
                for (int f = 0; f < 6; ++f)
                {
                    tile[f][0] = BLOCKS[id].faceTiles[f].x;
                    tile[f][1] = BLOCKS[id].faceTiles[f].y;
                }

                // Check each face and add a quad if exposed
                // faceDir: 0=-Z, 1=+Z, 2=-X, 3=+X, 4=-Y, 5=+Y
                if (z == 0 || blocks[x][y][z - 1] == 0)
                    addQuad(bx, by, bz, bx + 1, by + 1, bz, tile[0][0], tile[0][1], false, 0); // -Z (Front)
                if (z == CHUNK_SIZE - 1 || blocks[x][y][z + 1] == 0)
                    addQuad(bx, by, bz + 1, bx + 1, by + 1, bz + 1, tile[1][0], tile[1][1], false, 1); // +Z (Back)
                if (x == 0 || blocks[x - 1][y][z] == 0)
                    addQuad(bx, by, bz, bx, by + 1, bz + 1, tile[2][0], tile[2][1], false, 2); // -X (Left)
                if (x == CHUNK_SIZE - 1 || blocks[x + 1][y][z] == 0)
                    addQuad(bx + 1, by, bz, bx + 1, by + 1, bz + 1, tile[3][0], tile[3][1], false, 3); // +X (Right)
                if (y == 0 || blocks[x][y - 1][z] == 0)
                    addQuad(bx, by, bz, bx + 1, by, bz + 1, tile[4][0], tile[4][1], false, 4); // -Y (Bottom)
                if (y == CHUNK_HEIGHT - 1 || blocks[x][y + 1][z] == 0)
                    addQuad(bx, by + 1, bz, bx + 1, by + 1, bz + 1, tile[5][0], tile[5][1], false, 5); // +Y (Top)
            }
        }
    }

    vertexCount = vertexData.size() / 6; // 3 positions, 2 UVs, 1 brightness = 6 floats per vertex

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    // std::cout << "BuildMeshData (Applied 180-degree rotation to all faces) took " << elapsed.count() << " ms\n";
}

void Chunk::UploadMesh()
{
    if (vertexData.empty())
        return;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // Attribute 0: Position (vec3) — x, y, z (floats 0,1,2)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Attribute 1: UV (vec2) — u, v (floats 3,4)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Attribute 2: Brightness (float) — light level (float 5)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Chunk::Draw(Shader &shader) const
{
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    shader.SetUniform1i("u_Texture", 0);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

int Chunk::GetBlock(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE)
    {
        // std::cout << "OUT OF RANGE (GET BLOCK)" << std::endl;
        // std::cout << x << ", " << y << ", " << z << ", " << std::endl;
        return 0;
    }
    return blocks[x][y][z];
}

void Chunk::SetBlock(int x, int y, int z, int blockID)
{
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE)
    {
        // std::cout << "OUT OF RANGE (SET BLOCK)" << std::endl;
        return;
    }
    blocks[x][y][z] = blockID;
}