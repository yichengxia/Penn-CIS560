#pragma once
#include "drawable.h"
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char {
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, BRONZE
};

// The six cardinal directions in 3D space
enum Direction : unsigned char {
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

struct VertexData {
    glm::vec4 pos;
    glm::vec2 uv;
    VertexData(const glm::vec4 &p, const glm::vec2 &u);
};

struct BlockFace {
    Direction direction;
    glm::vec3 directionVec;
    std::array<VertexData, 4> vertices;
    BlockFace(Direction dir, const glm::vec3 &dirV, const std::array<VertexData, 4> &v);
};

#define BLK_UV 0.03125f

const std::array<BlockFace, 6> adjacentFaces = {
    BlockFace(XPOS, glm::vec3(1, 0, 0),
              {VertexData({1, 0, 1, 0}, {0, 0}),
               VertexData({1, 0, 0, 0}, {BLK_UV, 0}),
               VertexData({1, 1, 0, 0}, {BLK_UV, BLK_UV}),
               VertexData({1, 1, 1, 0}, {0, BLK_UV})}),

    BlockFace(XNEG, glm::vec3(-1, 0, 0),
              {VertexData({0, 0, 0, 0}, {0, 0}),
               VertexData({0, 0, 1, 0}, {BLK_UV, 0}),
               VertexData({0, 1, 1, 0}, {BLK_UV, BLK_UV}),
               VertexData({0, 1, 0, 0}, {0, BLK_UV})}),

    BlockFace(YPOS, glm::vec3(0, 1, 0),
              {VertexData({0, 1, 1, 0}, {0, 0}),
               VertexData({1, 1, 1, 0}, {BLK_UV, 0}),
               VertexData({1, 1, 0, 0}, {BLK_UV, BLK_UV}),
               VertexData({0, 1, 0, 0}, {0, BLK_UV})}),

    BlockFace(YNEG, glm::vec3(0, -1, 0),
              {VertexData({0, 0, 0, 0}, {0, 0}),
               VertexData({1, 0, 0, 0}, {BLK_UV, 0}),
               VertexData({1, 0, 1, 0}, {BLK_UV, BLK_UV}),
               VertexData({0, 0, 1, 0}, {0, BLK_UV})}),

    BlockFace(ZPOS, glm::vec3(0, 0, 1),
              {VertexData({0, 0, 1, 0}, {0, 0}),
               VertexData({1, 0, 1, 0}, {BLK_UV, 0}),
               VertexData({1, 1, 1, 0}, {BLK_UV, BLK_UV}),
               VertexData({0, 1, 1, 0}, {0, BLK_UV})}),

    BlockFace(ZNEG, glm::vec3(0, 0, -1),
              {VertexData({1, 0, 0, 0}, {0, 0}),
               VertexData({0, 0, 0, 0}, {BLK_UV, 0}),
               VertexData({0, 1, 0, 0}, {BLK_UV, BLK_UV}),
               VertexData({1, 1, 0, 0}, {0, BLK_UV})}),
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
    // Helper function that check if BlockType is empty
    bool isOpaque(BlockType t);
    // Helper function to get block color
    glm::vec4 getColor(BlockType t);

public:
    Chunk(OpenGLContext* mp_context);
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    virtual void createVBOdata() override;
};
