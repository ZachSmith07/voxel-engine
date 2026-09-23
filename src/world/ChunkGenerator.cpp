#include <chrono>
#include <iostream>
#include "FastNoiseLite.h"
#include "ChunkGenerator.h"
#include <thread>
#include <vector>
#include <random>

class WorldNoise
{
public:
    FastNoiseLite continentalNoise;
    FastNoiseLite erosionNoise;
    FastNoiseLite peaksValleysNoise;
    FastNoiseLite lakesNoise;
    FastNoiseLite dimensionNoise;
    FastNoiseLite shoreNoise;

    WorldNoise(int seed = 0)
    {
        continentalNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        continentalNoise.SetSeed(seed + 0);
        continentalNoise.SetFrequency(0.003f);

        erosionNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        erosionNoise.SetSeed(seed + 999);
        erosionNoise.SetFrequency(0.04f);

        peaksValleysNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        peaksValleysNoise.SetSeed(seed + 1382);
        peaksValleysNoise.SetFrequency(0.03f);

        lakesNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        lakesNoise.SetSeed(seed + 2981);
        lakesNoise.SetFrequency(0.3f);

        dimensionNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        dimensionNoise.SetSeed(seed + 499);
        dimensionNoise.SetFrequency(0.05f);

        shoreNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        shoreNoise.SetSeed(seed + 4321);
        shoreNoise.SetFrequency(0.01f);
    }
};

bool GenerateBool(double probability, unsigned int seed)
{
    std::mt19937 gen(seed); // Seeded Mersenne Twister RNG
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen) < probability;
}

struct TerrainPoint
{
    float continentalness;
    float height;
};

struct ErosionPoint
{
    float erosion;
    float smoothness;
};

struct HillsValleysPoint
{
    float hillsValleys;
    float terrainChange;
};

struct LakesPoint
{
    float lake;
    float terrainChange;
};

const std::vector<TerrainPoint> continentalnessValues = {
    {-1.0f, 90.0f},
    {-0.25f, 115.0f},
    {-0.1f, 122.0f},
    {0.15f, 135.0f},
    {0.8f, 140.0f},
    {1.0f, 165.0f}};

// const std::vector<ErosionPoint> erosionValues = {
//     {-1.0f, 0.07f},
//     {-0.5f, 0.08f},
//     {-0.2f, 0.09f},
//     {0.1f, 0.1f},
//     {0.6f, 0.18f},
//     {1.0f, 0.25f}};

const std::vector<ErosionPoint> erosionValues = {
    {0.0f, 1.0f},
    {0.25f, 0.08f},
    {0.45f, 0.06f},
    {1.0f, 0.05f}};

const std::vector<HillsValleysPoint> hillsValleysValues = {
    {0.0f, -25.0f},
    {0.02f, -20.0f},
    {0.04f, -10.0f},
    {0.08f, -2.0f},
    {0.5f, 2.0f},
    {0.54f, 12.0f},
    {0.58f, 22.0f},
    {1.0f, 35.0f}};

const std::vector<LakesPoint> lakesValues = {
    {0.0f, 0.0f},
    {0.94f, 0.0f},
    {0.96f, -5.0f},
    {0.98f, -25.0f},
    {1.0f, -35.0f}};

template <typename Point>
float Interpolate(float input, const std::vector<Point> &points,
                  float Point::*xField, float Point::*yField)
{
    input = std::max((points.front().*xField), std::min((points.back().*xField), input));

    for (size_t i = 1; i < points.size(); ++i)
    {
        const Point &left = points[i - 1];
        const Point &right = points[i];

        if (input <= right.*xField)
        {
            float t = (input - left.*xField) / (right.*xField - left.*xField);
            return left.*yField + t * (right.*yField - left.*yField);
        }
    }

    return points.back().*yField;
}

float GetBaseTerrainHeight(float continentalness)
{
    return Interpolate<TerrainPoint>(continentalness, continentalnessValues, &TerrainPoint::continentalness, &TerrainPoint::height);
}
float GetSmoothnessFromErosion(float erosion)
{
    return Interpolate<ErosionPoint>(abs(erosion), erosionValues, &ErosionPoint::erosion, &ErosionPoint::smoothness);
}
float GetTerrainHeightChangeFromHillsValleys(float hillsValleys)
{
    return Interpolate<HillsValleysPoint>(abs(hillsValleys), hillsValleysValues, &HillsValleysPoint::hillsValleys, &HillsValleysPoint::terrainChange);
}
float GetTerrainHeightChangeFromLake(float lake)
{
    return Interpolate<LakesPoint>(abs(lake), lakesValues, &LakesPoint::lake, &LakesPoint::terrainChange);
}

int GenerateRandomInt(int min, int max, unsigned int seed)
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

float GenerateRandomFloat(float min, float max, unsigned int seed)
{
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

bool IsSphereInsideChunk(float cx, float cy, float cz, float radius)
{
    return !(cx + radius < 0 || cx - radius > 15 ||
             cy + radius < 0 || cy - radius > 255 ||
             cz + radius < 0 || cz - radius > 15);
}

void RemoveSphere(int cx, int cy, int cz, BlockArray &blocks, float radius)
{
    if (IsSphereInsideChunk(cx, cy, cz, radius))
    {
        int r = static_cast<int>(std::ceil(radius));

        for (int x = cx - r; x <= cx + r; ++x)
        {
            if (x < 0 || x > 15)
                continue;

            for (int y = cy - r; y <= cy + r; ++y)
            {
                if (y < 0 || y > 255)
                    continue;

                for (int z = cz - r; z <= cz + r; ++z)
                {
                    if (z < 0 || z > 15)
                        continue;

                    float dx = static_cast<float>(x - cx);
                    float dy = static_cast<float>(y - cy);
                    float dz = static_cast<float>(z - cz);

                    if (dx * dx + dy * dy + dz * dz <= radius * radius)
                    {
                        if (blocks[x][y][z] != 5)
                        {
                            blocks[x][y][z] = 0;
                        }
                    }
                }
            }
        }
    }
}

// void GenerateCaves(BlockArray &blocks, int posX, int posZ)
// {
//     FastNoiseLite noise;
//     noise.SetSeed(17);                                         // Use seed 17
//     noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2); // Smooth, cave-like
//     noise.SetFrequency(0.03f);
//     FastNoiseLite noise2;
//     noise.SetSeed(1782);                                       // Use seed 17
//     noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2); // Smooth, cave-like
//     noise.SetFrequency(0.02f);                                 // Controls scale of features

//     for (int x = 0; x < 16; ++x)
//     {
//         float nx = static_cast<float>(x + posX * 16);
//         for (int z = 0; z < 16; ++z)
//         {
//             float nz = static_cast<float>(z + posZ * 16);
//             for (int y = 0; y < 256; ++y)
//             {
//                 float ny = static_cast<float>(y);

//                 // Normalize coordinates if desired, or scale for frequency
//                 float noiseValue1 = noise.GetNoise(nx, ny, nz); // Range [-1, 1]
//                 float noiseValue2 = noise2.GetNoise(nx, ny, nz);

//                 // if (y > 100)
//                 // {
//                 //     noiseValue -= (y - 100) * 0.005f;
//                 // }

//                 // Threshold determines where caves form
//                 if (noiseValue1 < -0.5f && noiseValue2 > 0.5f)
//                 {
//                     if (blocks[x][y][z] != 5)
//                     {
//                         blocks[x][y][z] = 3; // Air (carve cave)
//                     }
//                 }
//             }
//         }
//     }
// }

void GenerateCaves(BlockArray &blocks, int chunkX, int chunkZ)
{
    // NOISE SETUP
    FastNoiseLite worley;
    worley.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Cellular);
    worley.SetCellularReturnType(FastNoiseLite::CellularReturnType::CellularReturnType_Distance);
    worley.SetFrequency(0.05f);
    worley.SetSeed(1337);

    FastNoiseLite perlinX, perlinY, perlinZ;

    const float distortionAmp = 8.0f;
    const float distortionFreq = 0.05f * distortionAmp;

    perlinX.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
    perlinX.SetFrequency(distortionFreq);
    perlinX.SetSeed(101);

    perlinY.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
    perlinY.SetFrequency(distortionFreq);
    perlinY.SetSeed(102);

    perlinZ.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
    perlinZ.SetFrequency(distortionFreq);
    perlinZ.SetSeed(103);

    FastNoiseLite caveFloor;
    caveFloor.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    caveFloor.SetSeed(912);

    const float threshold = 0.53f;

    // GENERATE
    for (int localX = 0; localX < 16; ++localX)
    {
        int worldX = chunkX * 16 + localX;
        for (int localZ = 0; localZ < 16; ++localZ)
        {
            int worldZ = chunkZ * 16 + localZ;
            float dx = perlinX.GetNoise((float)worldX, (float)worldZ);
            float dz = perlinZ.GetNoise((float)worldX, (float)worldZ);
            int minY = caveFloor.GetNoise((float)worldX, (float)worldZ) + 3;
            float dy = perlinY.GetNoise((float)worldX, (float)minY, (float)worldZ);
            for (int y = minY; y < 120; ++y)
            {
                if (blocks[localX][y][localZ] != 0 && blocks[localX][y][localZ] != 5)
                {

                    if (y % 3 == 0)
                    {
                        dy = perlinY.GetNoise((float)worldX, (float)y, (float)worldZ);
                    }

                    float sampleX = worldX + dx;
                    float sampleY = y + dy;
                    float sampleZ = worldZ + dz;

                    float value = abs(worley.GetNoise(sampleX, sampleY, sampleZ));
                    if (y > 110)
                    {
                        value += (y - 110) * 0.05f;
                    }
                    // if (y > 124)
                    // {
                    //     value -= (y - 124) * 0.05f;
                    // }

                    if (value < threshold)
                    {
                        blocks[localX][y][localZ] = 0; // mark cave
                    }
                }
            }
        }
    }
}

void SpawnOres(BlockArray &blocks, int minVeins, int maxVeins, int minVeinLength, int maxVeinLength, int blockNum, unsigned int seedN, int minY, int maxY)
{
    int seed = seedN + blockNum * 72;

    int numVeins = GenerateRandomInt(minVeins, maxVeins, seed);
    for (int i = 0; i < numVeins; ++i)
    {
        int x = GenerateRandomInt(0, 15, seed + i * 82 + 1);
        int z = GenerateRandomInt(0, 15, seed + i * 921 + 92);
        int y = (GenerateRandomInt(minY, maxY, seed + i * 23 + 43) + GenerateRandomInt(minY, maxY, seed + i * 23 + 44)) / 2;

        int veinLength = GenerateRandomInt(minVeinLength, maxVeinLength, seed + i * 21 + 8);
        for (int j = 0; j < veinLength; ++j)
        {
            if (x >= 0 && x < 16 && y >= 0 && y < 256 && z >= 0 && z < 16)
            {
                if (blocks[x][y][z] == 3)
                {
                    blocks[x][y][z] = blockNum;
                }
            }

            int direction = GenerateRandomInt(0, 5, seed + i * 91 + 8 + j * 24);
            switch (direction)
            {
            case 0:
                ++x;
                break;
            case 1:
                --x;
                break;
            case 2:
                ++y;
                break;
            case 3:
                --y;
                break;
            case 4:
                ++z;
                break;
            case 5:
                --z;
                break;
            }
        }
    }
}

void GenerateMinerals(BlockArray &blocks, int chunkX, int chunkZ)
{
    unsigned int seed = (chunkX + 1000000) * 821 + (chunkZ + 1000000) * 792 + 0;

    SpawnOres(blocks, 4, 8, 8, 14, 6, seed, 60, 180); // COAL
    SpawnOres(blocks, 0, 6, 6, 10, 7, seed, 60, 130); // IRON
    SpawnOres(blocks, 1, 3, 0, 8, 8, seed, 10, 80);   // GOLD
    SpawnOres(blocks, 0, 1, -4, 6, 9, seed, 3, 50);   // DIAMOND
    SpawnOres(blocks, 1, 3, 2, 10, 10, seed, 10, 60); // REDSTONE
    SpawnOres(blocks, 1, 3, 0, 8, 11, seed, 3, 50);   // LAPIZ
}

void GenerateSpag(BlockArray &blocks, int posX, int posZ, const FastNoiseLite &continentalNoise)
{
    for (int x = posX - 2; x <= posX + 2; ++x)
    {
        for (int z = posZ - 2; z <= posZ + 2; ++z)
        {
            if (continentalNoise.GetNoise((float)(x * 16), (float)(z * 16)) > 0.15f)
            {
                unsigned int seed = (x + 1000000) * 892 + (z + 1000000) * 9243 + 0; // 0 is seed
                bool generateCave = GenerateBool(0.2, seed);
                if (generateCave)
                {
                    int xStart = GenerateRandomInt(0, 15, seed + 12147);
                    int yStart = (GenerateRandomInt(120, 145, seed + 259783) + GenerateRandomInt(120, 145, seed + 382834)) / 2;
                    int zStart = GenerateRandomInt(0, 15, seed + 378452);
                    int localX = (x - posX) * 16 + xStart;
                    int localZ = (z - posZ) * 16 + zStart;

                    int steps = GenerateRandomInt(10, 16, seed + 823);

                    float radius = GenerateRandomInt(30, 45, seed + 28432);
                    radius = radius / 10;

                    glm::vec3 directionPos = glm::vec3(GenerateRandomFloat(-1, 1, seed + 8943), GenerateRandomFloat(-1.5f, -1.0f, seed + 7341), GenerateRandomFloat(-1, 1, seed + 2314));
                    directionPos = glm::normalize(directionPos);
                    glm::vec3 directionNeg = -directionPos;

                    glm::vec3 posPos = glm::vec3(localX, yStart, localZ);
                    glm::vec3 negPos = glm::vec3(localX, yStart, localZ);

                    for (int i = 0; i < steps; ++i)
                    {
                        posPos += directionPos * 4.0f;
                        RemoveSphere(posPos.x, posPos.y, posPos.z, blocks, radius);
                        glm::vec3 directionOffset = glm::vec3(GenerateRandomFloat(-1, 1, seed + i * 13564), GenerateRandomFloat(-1, 1, seed + i * 22335), GenerateRandomFloat(-1, 1, seed + i * 32314));
                        directionOffset = glm::normalize(directionOffset) / 4.0f;
                        directionPos += directionOffset;
                        directionPos = glm::normalize(directionPos);

                        negPos += directionNeg * 4.0f;
                        RemoveSphere(negPos.x, negPos.y, negPos.z, blocks, radius);
                        directionOffset = glm::vec3(GenerateRandomFloat(-1, 1, seed + i * 1352), GenerateRandomFloat(-1, 1, seed + i * 324), GenerateRandomFloat(-1, 1, seed + i * 3664));
                        directionOffset = glm::normalize(directionOffset) / 4.0f;
                        directionNeg += directionOffset;
                        directionNeg = glm::normalize(directionPos);

                        radius += GenerateRandomFloat(-1, 1, seed + i * 72 + x * 7 + z * 9);
                    }

                    RemoveSphere(localX, yStart, localZ, blocks, radius);

                    // RemoveSphere(xStart, yStart, zStart, blocks, 7.0f);
                }
            }
        }
    }
}

int mod(int a, int b)
{
    return ((a % b) + b) % b;
}
int GetChunkCoord(int worldCoord, int chunkSize = 16)
{
    return (worldCoord >= 0) ? (worldCoord / chunkSize) : ((worldCoord - chunkSize + 1) / chunkSize);
}

void GenerateTree(BlockArray &blocks, int posX, int posZ, int baseX, int baseY, int baseZ)
{
    unsigned int seed = (posX + 1000000) * 821 + (posZ + 1000000) * 918 + baseX * 83 + baseY * 91 + baseZ * 24;

    int trunkHeight = GenerateRandomInt(5, 8, seed);
    int branches = GenerateRandomInt(0, 2, seed + 82);

    int x = baseX;
    int y = baseY;
    int z = baseZ;

    for (int i = 0; i < trunkHeight; ++i)
    {
        if (x >= 0 && x < 16 && z >= 0 && z < 16 && y >= 0 && z < 256)
        {
            blocks[x][y][z] = 12;
        }
        int direction = GenerateRandomInt(0, 12, seed + 94 + i * 81);
        switch (direction)
        {
        case 0:
            x += 1;
        case 1:
            x -= 1;
        case 2:
            z += 1;
        case 3:
            z -= 1;
        }
        y += 1;
    }

    int radius = GenerateRandomInt(1 + (trunkHeight - 5) / 2, 3 + +(trunkHeight - 5) / 2, seed + 912);
    for (int rx = -radius; rx <= radius; ++rx)
    {
        for (int ry = -radius; ry <= radius; ++ry)
        {
            for (int rz = -radius; rz <= radius; ++rz)
            {
                if (rx * rx + ry * ry + rz * rz <= radius * radius)
                {
                    float val = pow(radius * radius - rx * rx + ry * ry + rz * rz, 2);
                    if (GenerateRandomFloat(0, 1, seed + 382 + rx * 9 + ry * 5 + rz * 3) < val)
                    {
                        int xPlace = rx + x;
                        int yPlace = ry + y;
                        int zPlace = rz + z;
                        if (xPlace >= 0 && xPlace < 16 && yPlace >= 0 && yPlace < 256 && zPlace >= 0 && zPlace < 16)
                        {
                            if (blocks[xPlace][yPlace][zPlace] == 0)
                            {
                                blocks[xPlace][yPlace][zPlace] = 13;
                            }
                        }
                    }
                }
            }
        }
    }
}

float GetDensity(int x, int y, int z, const WorldNoise &worldNoise)
{
    const int SEA_LEVEL = 120;

    float xP = static_cast<float>(x);
    float yP = static_cast<float>(y);
    float zP = static_cast<float>(z);

    float continentalNoiseValue = worldNoise.continentalNoise.GetNoise(xP, zP);
    float baseHeight = GetBaseTerrainHeight(continentalNoiseValue);

    float peakValleysNoiseValue = worldNoise.peaksValleysNoise.GetNoise(xP, zP);
    float heightOffset = GetTerrainHeightChangeFromHillsValleys(continentalNoiseValue);
    baseHeight += heightOffset;

    float lakesNoiseValue = worldNoise.lakesNoise.GetNoise(xP, zP);
    float lakesOffset = GetTerrainHeightChangeFromLake(lakesNoiseValue);
    baseHeight += lakesOffset;

    float erosionNoiseValue = worldNoise.erosionNoise.GetNoise(xP, zP);
    float erosionValue = GetSmoothnessFromErosion(erosionNoiseValue);

    float density = worldNoise.dimensionNoise.GetNoise(xP, yP, zP);
    density += (baseHeight - yP) * erosionValue;

    return density;
}

// int GetSurfaceHeight(int posX, int posZ, const WorldNoise &worldNoise)
// {
//     for (int y = CHUNK_HEIGHT - 1; y >= 120; --y)
//     {
//         float density = GetDensity(posX, y, posZ, worldNoise);
//         if (density > 0)
//         {
//             return y;
//             break;
//         }
//     }
//     return 0;
// }

void GenerateTrees(BlockArray &blocks, int posX, int posZ, const WorldNoise &worldNoise)
{
    auto start = std::chrono::high_resolution_clock::now(); // Start timing

    FastNoiseLite treesNoise;
    treesNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    treesNoise.SetFrequency(0.01f);

    float noiseValue = treesNoise.GetNoise((float)(posX * 16), (float)(posZ * 16));
    noiseValue = noiseValue;

    int minTrees = abs(noiseValue) * 18;
    int maxTrees = abs(noiseValue) * 36;

    unsigned int seed = (posX + 1000000) * 821 + (posZ + 1000000) * 918;

    int nOfTrees = GenerateRandomInt(minTrees, maxTrees, seed);

    for (int i = 0; i < nOfTrees; ++i)
    {
        int x = GenerateRandomInt(3, 12, seed + 29 + i * 81);
        int z = GenerateRandomInt(3, 12, seed + 82 + i * 96);

        for (int y = 180; y >= 120; --y)
        {
            if (blocks[x][y][z] != 0)
            {
                if (blocks[x][y][z] == 1 || blocks[x][y][z] == 2)
                {
                    GenerateTree(blocks, posX, posZ, x, y, z);
                }
                break;
            }
        }
    }
}

// int GetBlock(int x, int y, int z, const FastNoiseLite &continentalNoise, const FastNoiseLite &erosionNoise, const FastNoiseLite &dimensionNoise)
// {
//     const int SEA_LEVEL = 120;
//     float xP = static_cast<float>(x);
//     float yP = static_cast<float>(y);
//     float zP = static_cast<float>(z);
//     float continentalNoiseValue = continentalNoise.GetNoise(xP, zP);
//     float baseHeight = GetBaseTerrainHeight(continentalNoiseValue);
//     float erosionNoiseValue = erosionNoise.GetNoise(xP, zP);
//     float erosionValue = GetSmoothnessFromErosion(erosionNoiseValue);

//     float density = dimensionNoise.GetNoise(xP, yP, zP);
//     density += (baseHeight - yP) * erosionValue;

//     if (density > 0)
//     {
//         return 3;
//     }
//     else if (y < SEA_LEVEL)
//     {
//         return 5;
//     }
//     else
//     {
//         return 0;
//     }

// }

// void GenerateChunk(BlockArray &blocks, int posX, int posZ)
// {
//     auto start = std::chrono::high_resolution_clock::now();

//     // Prepare noise generators once
//     FastNoiseLite continentalNoise;
//     continentalNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     continentalNoise.SetSeed(0);
//     continentalNoise.SetFrequency(0.003f);

//     FastNoiseLite eroisionNoise;
//     eroisionNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     eroisionNoise.SetSeed(999);
//     eroisionNoise.SetFrequency(0.0015f);

//     FastNoiseLite dimensionNoise;
//     dimensionNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     dimensionNoise.SetSeed(499);

//     const int NUM_THREADS = std::thread::hardware_concurrency(); // Use all cores
//     const int SLICE = CHUNK_SIZE / NUM_THREADS;                  // Number of Z-slices per thread

//     std::vector<std::thread> threads;

//     for (int t = 0; t < NUM_THREADS; ++t)
//     {
//         int zStart = t * SLICE;
//         int zEnd = (t == NUM_THREADS - 1) ? CHUNK_SIZE : zStart + SLICE;

//         threads.emplace_back([=, &blocks]()
//                              {
//             for (int x = 0; x < CHUNK_SIZE; ++x)
//             {
//                 for (int y = 0; y < CHUNK_HEIGHT; ++y)
//                 {
//                     for (int z = zStart; z < zEnd; ++z)
//                     {
//                         int worldX = x + posX * CHUNK_SIZE;
//                         int worldZ = z + posZ * CHUNK_SIZE;

//                         blocks[x][y][z] = GetBlock(worldX, y, worldZ, continentalNoise, eroisionNoise, dimensionNoise);
//                     }
//                 }
//             } });
//     }

//     // Wait for all threads to finish
//     for (auto &thread : threads)
//     {
//         thread.join();
//     }

//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double, std::milli> duration = end - start;
//     std::cout << "GenerateChunk (threaded) took " << duration.count() << " ms\n";
// }

void GenerateChunk(BlockArray &blocks, int posX, int posZ)
{
    auto start = std::chrono::high_resolution_clock::now();

    WorldNoise worldNoise;

    // FastNoiseLite caveNoise;
    // caveNoise.SetSeed(7);
    // caveNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    // caveNoise.SetFrequency(0.007f);

    // FastNoiseLite caveNoise2;
    // caveNoise.SetSeed(14);
    // caveNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    // caveNoise.SetFrequency(0.007f);

    const int SEA_LEVEL = 120;

    auto startTerrain = std::chrono::high_resolution_clock::now();

    const int NUM_THREADS = std::thread::hardware_concurrency();
    const int SLICE = CHUNK_SIZE / NUM_THREADS;

    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        int zStart = t * SLICE;
        int zEnd = (t == NUM_THREADS - 1) ? CHUNK_SIZE : zStart + SLICE;

        threads.emplace_back([=, &blocks, &worldNoise]() {

            float shoreCache[CHUNK_SIZE][CHUNK_SIZE];
            for (int x = 0; x < CHUNK_SIZE; ++x)
            {
                for (int z = zStart; z < zEnd; ++z)
                {
                    int worldX = x + posX * CHUNK_SIZE;
                    int worldZ = z + posZ * CHUNK_SIZE;
                    float shoreVal = worldNoise.shoreNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
                    shoreCache[x][z] = (shoreVal + 1.0f) * 0.5f;
                }
            }

            for (int x = 0; x < CHUNK_SIZE; ++x)
            {
                for (int z = zStart; z < zEnd; ++z)
                {
                    int worldX = x + posX * CHUNK_SIZE;
                    int worldZ = z + posZ * CHUNK_SIZE;

                    float shoreValue = shoreCache[x][z];
                    int maxSandHeight = static_cast<int>(shoreValue * 7) - 3;

                    bool surfaceFound = false;
                    int surfaceY = -1;

                    for (int y = CHUNK_HEIGHT - 1; y >= 0; --y)
                    {
                        if (y > 190)
                        {
                            blocks[x][y][z] = 0;
                            continue;
                        }

                        if (y < 70)
                        {
                            blocks[x][y][z] = 3; // deep stone
                            continue;
                        }

                        float density = GetDensity(worldX, y, worldZ, worldNoise);

                        if (density < 0.0f && y > SEA_LEVEL + 10)
                        {
                            blocks[x][y][z] = 0;
                            continue;
                        }

                        int blockType = 0;

                        if (density > 0)
                        {
                            if (!surfaceFound)
                            {
                                surfaceFound = true;
                                surfaceY = y;

                                if (y < SEA_LEVEL + maxSandHeight)
                                    blockType = 4; // sand
                                else
                                    blockType = 1; // grass
                            }
                            else
                            {
                                int depth = surfaceY - y;

                                float dirtNoise = worldNoise.dimensionNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(y), static_cast<float>(worldZ));
                                int dirtDepth = static_cast<int>(2 + ((surfaceY - SEA_LEVEL) * 0.1f) + (dirtNoise * 1.5f));

                                blockType = (depth <= dirtDepth) ? 2 : 3;
                            }
                        }
                        else if (y < SEA_LEVEL)
                        {
                            blockType = 5; // water
                        }

                        blocks[x][y][z] = blockType;
                    }
                }
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    auto endTerrain = std::chrono::high_resolution_clock::now();

    auto startMinerals = std::chrono::high_resolution_clock::now();
    GenerateMinerals(blocks, posX, posZ);
    auto endMinerals = std::chrono::high_resolution_clock::now();

    auto startCaves = std::chrono::high_resolution_clock::now();
    GenerateCaves(blocks, posX, posZ);
    auto endCaves = std::chrono::high_resolution_clock::now();

    auto startSpag = std::chrono::high_resolution_clock::now();
    GenerateSpag(blocks, posX, posZ, worldNoise.continentalNoise);
    auto endSpag = std::chrono::high_resolution_clock::now();

    auto startTrees = std::chrono::high_resolution_clock::now();
    GenerateTrees(blocks, posX, posZ, worldNoise);
    auto endTrees = std::chrono::high_resolution_clock::now();

    // std::cout << "GenerateTerrain took "
    //           << std::chrono::duration<double, std::milli>(endTerrain - startTerrain).count()
    //           << " ms\n";

    // std::cout << "GenerateMinerals took "
    //           << std::chrono::duration<double, std::milli>(endMinerals - startMinerals).count()
    //           << " ms\n";

    // std::cout << "GenerateCaves took "
    //           << std::chrono::duration<double, std::milli>(endCaves - startCaves).count()
    //           << " ms\n";

    // std::cout << "GenerateSpag took "
    //           << std::chrono::duration<double, std::milli>(endSpag - startSpag).count()
    //           << " ms\n";

    // std::cout << "GenerateTrees took "
    //           << std::chrono::duration<double, std::milli>(endTrees - startTrees).count()
    //           << " ms\n";

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    // std::cout << "GenerateChunk (threaded) took " << duration.count() << " ms\n";
}

// int GetBlock(int x, int y, int z,
//              const FastNoiseLite& noise,
//              const FastNoiseLite& cheeseCave,
//              const FastNoiseLite& cheeseCave2,
//              const FastNoiseLite& spaghettiCave1,
//              const FastNoiseLite& spaghettiCave2)
// {
//     const int SEA_LEVEL = 120;

//     float xP = static_cast<float>(x);
//     float yP = static_cast<float>(y);
//     float zP = static_cast<float>(z);

//     float cheeseNoise = cheeseCave.GetNoise(xP, yP * 1.4f, zP);
//     float cheeseNoise2 = cheeseCave2.GetNoise(xP, yP * 1.4f, zP);
//     float spaghettiNoise1 = abs(spaghettiCave1.GetNoise(xP, yP, zP));
//     float spaghettiNoise2 = abs(spaghettiCave2.GetNoise(xP, yP, zP));

//     if (yP > 95)
//     {
//         float adjust = (yP - 95) * 0.033f;
//         cheeseNoise += adjust;
//         cheeseNoise2 += adjust;
//     }
//     if (yP < 20)
//     {
//         float adjust = (20 - yP) * 0.031f;
//         cheeseNoise += adjust;
//         cheeseNoise2 += adjust;
//         spaghettiNoise1 += adjust;
//         spaghettiNoise2 += adjust;
//     }

//     float noiseLevel = noise.GetNoise(xP, yP, zP);
//     noiseLevel += (SEA_LEVEL - yP) * 0.012f;

//     if (noiseLevel > 0)
//     {
//         if (spaghettiNoise1 < 0.0f && spaghettiNoise2 < 0.0f)
//             return 0;
//         return 3;
//     }
//     else if (y < SEA_LEVEL)
//     {
//         return 5;
//     }
//     return 0;
// }

// void GenerateChunk(BlockArray &blocks, int posX, int posZ)
// {
//     auto start = std::chrono::high_resolution_clock::now();

//     // Prepare noise generators once
//     FastNoiseLite noise;
//     noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     noise.SetSeed(0);

//     FastNoiseLite cheeseCave;
//     cheeseCave.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     cheeseCave.SetSeed(999);

//     FastNoiseLite cheeseCave2;
//     cheeseCave2.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     cheeseCave2.SetSeed(1000);

//     FastNoiseLite spaghettiCave1;
//     spaghettiCave1.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     spaghettiCave1.SetSeed(1999);

//     FastNoiseLite spaghettiCave2;
//     spaghettiCave2.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
//     spaghettiCave2.SetSeed(827);

//     const int NUM_THREADS = std::thread::hardware_concurrency(); // Use all cores
//     const int SLICE = CHUNK_SIZE / NUM_THREADS;                  // Number of Z-slices per thread

//     std::vector<std::thread> threads;

//     for (int t = 0; t < NUM_THREADS; ++t)
//     {
//         int zStart = t * SLICE;
//         int zEnd = (t == NUM_THREADS - 1) ? CHUNK_SIZE : zStart + SLICE;

//         threads.emplace_back([=, &blocks]()
//                              {
//             for (int x = 0; x < CHUNK_SIZE; ++x)
//             {
//                 for (int y = 0; y < CHUNK_HEIGHT; ++y)
//                 {
//                     for (int z = zStart; z < zEnd; ++z)
//                     {
//                         int worldX = x + posX * CHUNK_SIZE;
//                         int worldZ = z + posZ * CHUNK_SIZE;

//                         blocks[x][y][z] = GetBlock(worldX, y, worldZ,
//                                                    noise, cheeseCave, cheeseCave2,
//                                                    spaghettiCave1, spaghettiCave2);
//                     }
//                 }
//             } });
//     }

//     // Wait for all threads to finish
//     for (auto &thread : threads)
//     {
//         thread.join();
//     }

//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double, std::milli> duration = end - start;
//     std::cout << "GenerateChunk (threaded) took " << duration.count() << " ms\n";
// }