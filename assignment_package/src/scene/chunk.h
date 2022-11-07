#pragma once
#include "chunkhelpers.h"
#include "drawable.h"
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>

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
    // Helper function that check if BlockType is empty
    bool isOpaque(BlockType t);
    // Helper function that takes in a vector of interleaved vertex data and a vector of index data,
    // and buffers them into the appropriate VBOs of Drawable
    void bufferVBOdata(std::vector<glm::vec4> interleaved, std::vector<int> idx);

public:
    explicit Chunk(OpenGLContext* mp_context);
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    BlockType getBlockAt(glm::vec3 pos) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    virtual void createVBOdata() override;
};
