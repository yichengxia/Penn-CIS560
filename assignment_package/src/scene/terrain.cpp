#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>
#include "noise_functions.h"
#include "chunkworkers.h"

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context), m_chunkCreated(0), m_tryExpansionTimer(0.f)
{}

Terrain::~Terrain() {}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for (int x = minX; x < maxX; x += 16) {
        for (int z = minZ; z < maxZ; z += 16) {
            // Insert a new Chunk into its map and set up its VBOs for rendering
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, 0, z)));
            shaderProgram->drawInterleaved(*chunk);
        }
    }
}

// unused in ms2
//// Checks whether a new Chunk should be added to the Terrain
//// based on the Player's proximity to the edge of a Chunk without a neighbor in a particular direction.
//// For milestone 1, when the player is 16 blocks of an edge of a Chunk that does not connect to an existing Chunk,
//// the Terrain should insert a new Chunk into its map and set up its VBOs for rendering.
void Terrain::generateTerrain(glm::vec3 pos) {
    auto chunkX = glm::floor(pos.x / 16.f) * 16, chunkZ = glm::floor(pos.z / 16.f) * 16;
    for (int x = chunkX - 64; x < chunkX + 65; x += 16) {
        for (int z = chunkZ - 64; z < chunkZ + 65; z += 16) {
            if (m_chunks.count(toKey(x, z)) == 0) {
                Chunk* c = instantiateChunkAt(x, z);
                c->fillChunk();
                c->createVBOdata();
                c->create(c->m_vboData.m_vboDataOpaque, c->m_vboData.m_idxDataOpaque,
                          c->m_vboData.m_vboDataTransparent, c->m_vboData.m_idxDataTransparent);
            }
        }
    }
}

// unused in ms2
void Terrain::CreateTestScene()
{
    // TODO: DELETE THIS LINE WHEN YOU DELETE m_geomCube!

    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            Chunk* c = instantiateChunkAt(x, z);
            c->fillChunk();
            c->createVBOdata();
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));
}

void Terrain::spawnVBOWorker(Chunk* chunkNeedingVBOData) {
    VBOWorker* worker = new VBOWorker(chunkNeedingVBOData, &m_chunksThatHaveVBOs, &m_chunksThatHaveVBOsLock);
    QThreadPool::globalInstance()->start(worker);
}

void Terrain::spawnFBMWorker(int64_t zone) {
    // For every terrain generation zone in this radius that does not yet exist in
    // Terrain's m_generatedTerrain, you will spawn a thread to fill that zone's
    // Chunks with procedural height field BlockType data.
    // We will designate these threads as BlockTypeWorkers.
    ivec2 coord = toCoords(zone);
    std::vector<Chunk*> chunksToFill;
    for(int x = coord.x; x < coord.x + 64; x += 16) {
        for(int z = coord.y; z < coord.y + 64; z += 16) {
            Chunk* c = instantiateChunkAt(x, z);
            chunksToFill.push_back(c);
        }
    }
    FBMWorker* worker = new FBMWorker(coord.x, coord.y, chunksToFill, &m_chunksThatHaveBlockData, &m_chunksThatHaveBlockDataLock);
    QThreadPool::globalInstance()->start(worker);
    m_generatedTerrain.insert(zone);
}

void Terrain::spawnVBOWorkers(const std::unordered_set<Chunk*> &chunksNeedingVBOs) {
    for (Chunk* c: chunksNeedingVBOs) {
        spawnVBOWorker(c);
    }
}

void Terrain::spawnFBMWorkers(const QSet<int64_t> &zonesToGenerate) {
    // Spawn worker threads to generate more Chunks
    for (int64_t zone : zonesToGenerate) {
        spawnFBMWorker(zone);
    }
}

void Terrain::checkThreadResults() {
    // Send Chunks that have been processed by FBMWorkers
    // to VBOWorkers for VBO data
    m_chunksThatHaveBlockDataLock.lock();
    spawnVBOWorkers(m_chunksThatHaveBlockData);
    m_chunksThatHaveBlockData.clear();
    m_chunksThatHaveBlockDataLock.unlock();

    // Collect the Chunks that have been given VBO data
    // by VBOWorkers and send that VBO data to the GPU
    m_chunksThatHaveVBOsLock.lock();
    for (ChunkVBOData &cd : m_chunksThatHaveVBOs) {
        cd.mp_chunk->create(cd.m_vboDataOpaque, cd.m_idxDataOpaque,
                            cd.m_vboDataTransparent, cd.m_idxDataTransparent);
    }
    if (m_chunkCreated < 25 * 4 * 4) {
        m_chunkCreated += m_chunksThatHaveVBOs.size();
    }
    m_chunksThatHaveVBOs.clear();
    m_chunksThatHaveVBOsLock.unlock();
}

QSet<int64_t> Terrain::terrainZonesBorderingZone(glm::ivec2 zone, unsigned int radius, bool onlyCircumference) const {
    int radiusInZoneScale = static_cast<int>(radius) * 64;
    QSet<int64_t> result;
    // Only want to look at terrain zones exactly at our radius
    if (onlyCircumference) {
        for (int i = - radiusInZoneScale; i < radiusInZoneScale; i += 64) {
            // Nx1 to the right
            result.insert(toKey(zone.x + radiusInZoneScale, zone.y + i));
            // Nx1 to the left
            result.insert(toKey(zone.x - radiusInZoneScale, zone.y + i));
            // Nx1 above
            result.insert(toKey(zone.x + i, zone.y + radiusInZoneScale));
            // Nx1 below
            result.insert(toKey(zone.x + i, zone.y - radiusInZoneScale));
        }
    } else {
        for (int i = -radiusInZoneScale; i <= radiusInZoneScale; i += 64) {
            for (int j = -radiusInZoneScale; j <= radiusInZoneScale; j += 64) {
                result.insert(toKey(zone.x + i, zone.y + j));
            }
        }
    }
    return result;
}

bool Terrain::terrainZoneExists(int64_t id) const {
    return m_generatedTerrain.count(id);
}

void Terrain::tryExpansion(glm::vec3 playerPos, glm::vec3 playerPosPrev) {
    // Find the player's position relative
    // to their current terrain gen zone
    ivec2 currZone(64.f * glm::floor(playerPos.x / 64.f), 64.f * glm::floor(playerPos.z / 64.f));
    ivec2 prevZone(64.f * glm::floor(playerPosPrev.x / 64.f), 64.f * glm::floor(playerPosPrev.z / 64.f));
    // Determine which terrain zones border our currect position and our previoius position
    // This *will* include un-generated terrain zones, so we can compare them to our gl...
    // and know to generate them
    QSet<int64_t> terrainZonesBorderingCurrPos = terrainZonesBorderingZone(currZone, TERRAIN_CREATE_RADIUS, false);
    QSet<int64_t> terrainZonesBorderingPrevPos = terrainZonesBorderingZone(prevZone, TERRAIN_CREATE_RADIUS, false);
    // Check which terrain zones need to be destroy()ed
    // by determining which terrain zones were previously in our radius and are not not
    for (auto id : terrainZonesBorderingPrevPos) {
        if (!terrainZonesBorderingCurrPos.contains(id)) {
            ivec2 coord = toCoords(id);
            for (int x = coord.x; x < coord.x + 64; x += 16) {
                for (int z = coord.y; z < coord.y + 64; z += 16) {
                    auto& chunk = getChunkAt(x, z);
                    chunk->destroyVBOdata();
                }
            }
        }
    }
    // Determine if any terrain zones around our current position need VBO data
    // Send these to VBOWorkers
    // DO NOT send zones to workers if they do not exist in our global map
    // Instead, send these to FBMWorkers.
    for (auto id : terrainZonesBorderingCurrPos) {
        // If it exists already AND IS NOT IN PREV SET, send it to a VBOWorker
        // If it's in the prev set, then it's already been sent to a VBOWorker
        // at some point, and may even already have VBOs
        if (terrainZoneExists(id)) {
            // For every terrain generation zone in this radius that does exist in m_generatedTerrain
            // check each Chunk it contains and see if it already has VBO data
            if (!terrainZonesBorderingPrevPos.contains(id)) {
                // If it does not, then you will spawn another thread
                // we will designate a VBOWorker to compute the interleaved buffer and index buffer data for that Chunk.
                ivec2 coord = toCoords(id);
                for (int x = coord.x; x < coord.x + 64; x += 16) {
                    for (int z = coord.y; z < coord.y + 64; z += 16) {
                        auto & chunk = getChunkAt(x, z);
                        spawnVBOWorker(chunk.get());
                    }
                }
            }
        } else {
            // If it does not yet exist, send it to an FBMWorker
            // This also adds it to the set of generated terrain zones
            // so we don't try to repeatedly generate it
            spawnFBMWorker(id);
        }
    }
}

void Terrain::multithreadedWork(glm::vec3 playerPos, glm::vec3 playerPosPrev, float dT) {
    m_tryExpansionTimer += dT;
    // Only check for terrain expansion every 0.5 second of real time or so
    if (m_tryExpansionTimer < 0.5f) {
        return;
    }
    tryExpansion(playerPos, playerPosPrev);
    checkThreadResults();
    m_tryExpansionTimer = 0.f;
}

bool Terrain::initialTerrainDoneLoading() {
    return m_chunkCreated >= 25 * 4 * 4;
}
