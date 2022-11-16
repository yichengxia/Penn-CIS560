#include "chunkworkers.h"

FBMWorker::FBMWorker(int x, int z, std::vector<Chunk*> chunksToFill,
                         std::unordered_set<Chunk*>* chunksCompleted, QMutex* ChunksCompletedLock) :
    m_xCorner(x), m_zCorner(z), m_chunksToFill(chunksToFill),
    mp_chunksCompleted(chunksCompleted), mp_chunksCompletedLock(ChunksCompletedLock)
{}

void FBMWorker::run() {
    for (auto &chunk : m_chunksToFill) {
        chunk->fillChunk();
        mp_chunksCompletedLock->lock();
        mp_chunksCompleted->insert(chunk);
        mp_chunksCompletedLock->unlock();
    }
}

VBOWorker::VBOWorker(Chunk* c, std::vector<ChunkVBOData>* dat, QMutex * datLock) :
    mp_chunk(c), mp_chunkVBOsCompleted(dat), mp_chunkVBOsCompletedLock(datLock)
{}

void VBOWorker::run() {
    mp_chunk->createVBOdata();

    mp_chunkVBOsCompletedLock->lock();
    mp_chunkVBOsCompleted->push_back(mp_chunk->m_vboData);
    mp_chunkVBOsCompletedLock->unlock();
}
