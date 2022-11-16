#pragma once
#include <glm/glm.hpp>
#include "chunk.h"
#include <QRunnable>
#include <QMutex>
#include <unordered_set>

// BlockTypeWorkers
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

};

class VBOWorker : public QRunnable {
private:
    Chunk* mp_chunk;
    std::vector<ChunkVBOData>* mp_chunkVBOsCompleted;
    QMutex *mp_chunkVBOsCompletedLock;
public:
    VBOWorker(Chunk* c, std::vector<ChunkVBOData>* dat, QMutex * datLock);
    void run() override;
};
