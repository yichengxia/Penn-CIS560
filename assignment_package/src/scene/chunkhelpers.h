#pragma once
#include "glm/glm.hpp"
#include <array>

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
    VertexData(const glm::vec4 &p, const glm::vec2 &u) : pos(p), uv(u) {}
};

struct BlockFace {
    Direction direction;
    glm::vec3 directionVec;
    std::array<VertexData, 4> vertices;
    BlockFace(Direction dir, const glm::vec3 &dirV, const std::array<VertexData, 4> &v) : direction(dir), directionVec(dirV), vertices(v) {}
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
