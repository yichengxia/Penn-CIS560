#pragma once
#include "chunkhelpers.h"
#include "drawable.h"
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include <vector>

class Chunk;

struct ChunkVBOData {
    Chunk* mp_chunk;
    std::vector<glm::vec4> m_vboDataOpaque, m_vboDataTransparent;
    std::vector<GLuint> m_idxDataOpaque, m_idxDataTransparent;

    ChunkVBOData(Chunk* c) :
        mp_chunk(c), m_vboDataOpaque{}, m_vboDataTransparent{},
        m_idxDataOpaque{}, m_idxDataTransparent{}
    {}
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;
    glm::ivec2 m_pos;
    // Helper function that check if BlockType is empty
    bool isOpaque(BlockType t);
    // Helper function to get block color
    glm::vec4 getColor(BlockType t);

public:
    ChunkVBOData m_vboData;
    Chunk(OpenGLContext* mp_context);
    Chunk(OpenGLContext* mp_context, int x, int z);
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    virtual void createVBOdata() override;
    void fillChunk();
    void create(std::vector<glm::vec4> m_vboDataOpaque, std::vector<GLuint>,
                std::vector<glm::vec4> m_vboDataTransparent, std::vector<GLuint> m_idxDataTransparent);
    void setMCount(int c);
};
