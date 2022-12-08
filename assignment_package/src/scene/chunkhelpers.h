#pragma once
#include "glm/glm.hpp"
#include <array>
#include <unordered_map>

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char {
    EMPTY, GRASS, DIRT, STONE, ICE, WATER, SNOW, BRONZE, LAVA, BEDROCK, SAND, TREE, OTHER,
    // For height map feature
    BLACK, WHITE, RED, LIME, BLUE, YELLOW, CYAN, MAGENTA, SILVER, GRAY, MAROON, OLIVE, GREEN, PURPLE, TEAL, NAVY
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

#define BLK_UV 0.0625f

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

const std::unordered_map<BlockType, std::array<glm::vec2, 6>> uvs {
    {GRASS, {glm::vec2(3.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(3.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(8.f * BLK_UV, 13.f * BLK_UV),
             glm::vec2(2.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(3.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(3.f * BLK_UV, 15.f * BLK_UV)}},
    {DIRT, {glm::vec2(2.f * BLK_UV, 15.f * BLK_UV),
            glm::vec2(2.f * BLK_UV, 15.f * BLK_UV),
            glm::vec2(2.f * BLK_UV, 15.f * BLK_UV),
            glm::vec2(2.f * BLK_UV, 15.f * BLK_UV),
            glm::vec2(2.f * BLK_UV, 15.f * BLK_UV),
            glm::vec2(2.f * BLK_UV, 15.f * BLK_UV)}},
    {STONE, {glm::vec2(1.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(1.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(1.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(1.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(1.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(1.f * BLK_UV, 15.f * BLK_UV)}},
    {ICE, {glm::vec2(3.f * BLK_UV, 11.f * BLK_UV),
           glm::vec2(3.f * BLK_UV, 11.f * BLK_UV),
           glm::vec2(3.f * BLK_UV, 11.f * BLK_UV),
           glm::vec2(3.f * BLK_UV, 11.f * BLK_UV),
           glm::vec2(3.f * BLK_UV, 11.f * BLK_UV),
           glm::vec2(3.f * BLK_UV, 11.f * BLK_UV)}},
    {WATER, {glm::vec2(13.f * BLK_UV, 3.f * BLK_UV),
             glm::vec2(13.f * BLK_UV, 3.f * BLK_UV),
             glm::vec2(13.f * BLK_UV, 3.f * BLK_UV),
             glm::vec2(13.f * BLK_UV, 3.f * BLK_UV),
             glm::vec2(13.f * BLK_UV, 3.f * BLK_UV),
             glm::vec2(13.f * BLK_UV, 3.f * BLK_UV)}},
    {SNOW, {glm::vec2(2.f * BLK_UV, 11.f * BLK_UV),
            glm::vec2(2.f * BLK_UV, 11.f * BLK_UV),
            glm::vec2(2.f * BLK_UV, 11.f * BLK_UV),
            glm::vec2(2.f * BLK_UV, 11.f * BLK_UV),
            glm::vec2(2.f * BLK_UV, 11.f * BLK_UV),
            glm::vec2(2.f * BLK_UV, 11.f * BLK_UV)}},
    {LAVA, {glm::vec2(13.f * BLK_UV, 1.f * BLK_UV),
            glm::vec2(13.f * BLK_UV, 1.f * BLK_UV),
            glm::vec2(13.f * BLK_UV, 1.f * BLK_UV),
            glm::vec2(13.f * BLK_UV, 1.f * BLK_UV),
            glm::vec2(13.f * BLK_UV, 1.f * BLK_UV),
            glm::vec2(13.f * BLK_UV, 1.f * BLK_UV)}},
    {BEDROCK, {glm::vec2(1.f * BLK_UV, 14.f * BLK_UV),
               glm::vec2(1.f * BLK_UV, 14.f * BLK_UV),
               glm::vec2(1.f * BLK_UV, 14.f * BLK_UV),
               glm::vec2(1.f * BLK_UV, 14.f * BLK_UV),
               glm::vec2(1.f * BLK_UV, 14.f * BLK_UV),
               glm::vec2(1.f * BLK_UV, 14.f * BLK_UV)}},
    {SAND, {glm::vec2(14.f * BLK_UV, 7.f * BLK_UV),
               glm::vec2(14.f * BLK_UV, 7.f * BLK_UV),
               glm::vec2(14.f * BLK_UV, 7.f * BLK_UV),
               glm::vec2(14.f * BLK_UV, 7.f * BLK_UV),
               glm::vec2(14.f * BLK_UV, 7.f * BLK_UV),
               glm::vec2(14.f * BLK_UV, 7.f * BLK_UV)}},
    {TREE, {glm::vec2(7.f * BLK_UV, 12.f * BLK_UV),
               glm::vec2(14.f * BLK_UV, 7.f * BLK_UV),
               glm::vec2(14.f * BLK_UV, 7.f * BLK_UV),
               glm::vec2(14.f * BLK_UV, 7.f * BLK_UV),
               glm::vec2(14.f * BLK_UV, 7.f * BLK_UV),
               glm::vec2(14.f * BLK_UV, 7.f * BLK_UV)}},
    // For height map feature
    {BLACK, {glm::vec2(12.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 15.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 15.f * BLK_UV)}},
    {WHITE, {glm::vec2(12.f * BLK_UV, 14.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 14.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 14.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 14.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 14.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 14.f * BLK_UV)}},
    {RED, {glm::vec2(12.f * BLK_UV, 13.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 13.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 13.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 13.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 13.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 13.f * BLK_UV)}},
    {LIME, {glm::vec2(12.f * BLK_UV, 12.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 12.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 12.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 12.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 12.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 12.f * BLK_UV)}},
    {BLUE, {glm::vec2(12.f * BLK_UV, 11.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 11.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 11.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 11.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 11.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 11.f * BLK_UV)}},
    {YELLOW, {glm::vec2(12.f * BLK_UV, 10.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 10.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 10.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 10.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 10.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 10.f * BLK_UV)}},
    {CYAN, {glm::vec2(12.f * BLK_UV, 9.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 9.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 9.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 9.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 9.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 9.f * BLK_UV)}},
    {MAGENTA, {glm::vec2(12.f * BLK_UV, 8.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 8.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 8.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 8.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 8.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 8.f * BLK_UV)}},
    {SILVER, {glm::vec2(12.f * BLK_UV, 7.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 7.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 7.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 7.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 7.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 7.f * BLK_UV)}},
    {GRAY, {glm::vec2(12.f * BLK_UV, 6.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 6.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 6.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 6.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 6.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 6.f * BLK_UV)}},
    {MAROON, {glm::vec2(12.f * BLK_UV, 5.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 5.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 5.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 5.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 5.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 5.f * BLK_UV)}},
    {OLIVE, {glm::vec2(12.f * BLK_UV, 4.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 4.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 4.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 4.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 4.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 4.f * BLK_UV)}},
    {GREEN, {glm::vec2(12.f * BLK_UV, 3.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 3.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 3.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 3.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 3.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 3.f * BLK_UV)}},
    {PURPLE, {glm::vec2(12.f * BLK_UV, 2.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 2.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 2.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 2.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 2.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 2.f * BLK_UV)}},
    {TEAL, {glm::vec2(12.f * BLK_UV, 1.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 1.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 1.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 1.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 1.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 1.f * BLK_UV)}},
    {NAVY, {glm::vec2(12.f * BLK_UV, 0.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 0.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 0.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 0.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 0.f * BLK_UV),
             glm::vec2(12.f * BLK_UV, 0.f * BLK_UV)}},
};
