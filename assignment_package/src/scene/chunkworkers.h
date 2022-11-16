#pragma once
#include <glm/glm.hpp>
#include "chunk.h"
#include <QRunnable>
#include <QMutex>
#include <unordered_set>

//enum Biome {
//    GRASSLAND, MOUNTAIN, DESERT, ISLAND
//};

class FBMWorker : public QRunnable {
private:
    // Coords of the terrain zone being generated
    int m_xCorner, m_zCorner;
    std::vector<Chunk*> m_chunksToFill;
    std::unordered_set<Chunk*>* mp_chunksCompleted;
    QMutex* mp_chunksCompletedLock;
public:
    FBMWorker(int x, int z, std::vector<Chunk*> chunksToFill,
              std::unordered_set<Chunk*>* chunksCompleted, QMutex* ChunksCompletedLock);
    void run() override;
//    Biome biomeMap(glm::vec2 val) const;
////    BlockType positionToBlockType(Chunk *c, glm::ivec3 pos, int maxHeight, Biome b, const un...);
//    glm::vec2 computeBiomeSlope(glm::ivec3 pos, Biome b) const;

//    std::vector<glm::ivec2> spawnTreeLocations(int x, int z) const; // Unused
//    std::vector<std::tuple<glm::ivec3, BlockType>> generateTree(glm::ivec3 location, Biome b) const;
//    bool canPlaceTree(Chunk *c, int x, int y, int z, Biome b) const;

//    std::vector<std::tuple<glm::ivec3, BlockType>> oakTree(glm::ivec3 location) const;
//    std::vector<std::tuple<glm::ivec3, BlockType>> pineTree(glm::ivec3 location) const;
//    std::vector<std::tuple<glm::ivec3, BlockType>> brichTree(glm::ivec3 location) const;

};

//bool isTransparent(BlockType t);

class VBOWorker : public QRunnable {
private:
    Chunk* mp_chunk;
    std::vector<ChunkVBOData>* mp_chunkVBOsCompleted;
    QMutex *mp_chunkVBOsCompletedLock;
public:
    VBOWorker(Chunk* c, std::vector<ChunkVBOData>* dat, QMutex * datLock);
    void run() override;
};
