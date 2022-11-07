#pragma once
#include "glm/glm.hpp"
#include <array>
#include <unordered_set>

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char {
    EMPTY, GRASS, DIRT, STONE, WATER
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
    VertexData(glm::vec4 p, glm::vec2 u) : pos(p), uv(u) {}
};

struct BlockFace {
    Direction direction;
    glm::vec3 directionVec;
    std::array<VertexData, 4> vertices;
    BlockFace(Direction dir, glm::vec3 dirV, const VertexData &a, const VertexData &b, const VertexData &c, const VertexData &d) : direction(dir), directionVec(dirV), vertices{a, b, c, d} {}
};

#define BLK_UV 0.03125f

const static std::array<BlockFace, 6> adjacentFaces {
    // +X
    BlockFace(XPOS, glm::vec3(1, 0, 0), VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(0, 0)),
                                        VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(BLK_UV, 0)),
                                        VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
                                        VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(0, BLK_UV))),
    // -X
    BlockFace(XNEG, glm::vec3(-1, 0, 0), VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)),
                                         VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(BLK_UV, 0)),
                                         VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
                                         VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0, BLK_UV))),
    // +Y
    BlockFace(YPOS, glm::vec3(0, 1, 0),  VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0, 0)),
                                         VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(BLK_UV, 0)),
                                         VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
                                         VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0, BLK_UV))),
    // -Y
    BlockFace(YNEG, glm::vec3(0, -1, 0), VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)),
                                         VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(BLK_UV, 0)),
                                         VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
                                         VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0, BLK_UV))),
    // +Z
    BlockFace(ZPOS, glm::vec3(0, 0, 1), VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0, 0)),
                                        VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(BLK_UV, 0)),
                                        VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
                                        VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0, BLK_UV))),
    // -Z
    BlockFace(ZNEG, glm::vec3(0, 0, -1), VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(0, 0)),
                                         VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(BLK_UV, 0)),
                                         VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
                                         VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(0, BLK_UV)))
};
