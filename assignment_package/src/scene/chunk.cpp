#include "chunk.h"
#include "noise_functions.h"

Chunk::Chunk(OpenGLContext* mp_context) : Drawable(mp_context), m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}, m_vboData(this)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

Chunk::Chunk(OpenGLContext* mp_context, int x, int z) :
    Drawable(mp_context), m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}},
    m_pos(glm::ivec2(x, z)), m_vboData(this)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}

const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

// Helper function that check if BlockType is empty
// Note: LAVA is set to be transparent becaue we want player to swim in it
bool Chunk::isOpaque(BlockType t) {
    return t != EMPTY && t != WATER && t != ICE && t!= LAVA;
}

// Helper function to get block color
glm::vec4 Chunk::getColor(BlockType t) {
    switch (t) {
        case EMPTY:
            return glm::vec4(0.f, 0.f, 0.f, 0.f);
        case GRASS:
            return glm::vec4(95.f, 159.f, 53.f, 1.f) / 255.f;
        case DIRT:
            return glm::vec4(121.f, 85.f, 58.f, 1.f) / 255.f;
        case STONE:
            return glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
        case WATER:
            return glm::vec4(0.f, 0.f, 0.75f, 1.f);
        case SNOW:
            return glm::vec4(1.f, 1.f, 1.f, 1.f);
        case BRONZE:
            return glm::vec4(0.5f, 0.2f, 0.f, 1.f);
        case LAVA:
            return glm::vec4(0.9f, 0.0f, 0.3f, 1.f);
        case BEDROCK:
            return glm::vec4(0.6f, 0.5f, 0.5f, 1.f);
        default:
            // Other block types are not yet handled, so we default to debug purple
            return glm::vec4(1.f, 0.f, 1.f, 1.f);
    }
}

int Chunk::elemCount2() {
    return m_count2;
}

void Chunk::generateIdx2() {
    m_idx2Generated = true;
    mp_context->glGenBuffers(1, &m_bufIdx2);
}

void Chunk::generatePos2() {
    m_pos2Generated = true;
    mp_context->glGenBuffers(1, &m_bufPos2);
}

bool Chunk::bindIdx2() {
    if (m_idx2Generated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx2);
    }
    return m_idx2Generated;
}

bool Chunk::bindPos2() {
    if (m_pos2Generated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos2);
    }
    return m_pos2Generated;
}

void Chunk::createVBOdata() {
    // Initialize vectors to store interleaved and indices
    std::vector<glm::vec4> interleaved, interleaved2;
    std::vector<GLuint> idx, idx2;

    unsigned count = 0, count2 = 0;
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            for (int y = 0; y < 256; y++) {
                BlockType currType = getBlockAt(x, y, z);
                if (currType != EMPTY) {
                    auto &&bufUsing = isOpaque(currType) ? interleaved : interleaved2;
                    auto &&idxUsing = isOpaque(currType) ? idx : idx2;
                    auto &&countUsing = isOpaque(currType) ? count : count2;
                    for (auto &&neighborFace : adjacentFaces) {
                        bool canRender = y == 0 || y == 256;
                        if (!canRender) {
                            glm::ivec3 nextPos{x + neighborFace.directionVec.x, y + neighborFace.directionVec.y, z + neighborFace.directionVec.z};
                            auto nextChunk = this;
                            if (nextPos.x >= 16) {
                                nextPos.x = 0;
                                nextChunk = m_neighbors[XPOS];
                            } else if (nextPos.x < 0) {
                                nextPos.x = 15;
                                nextChunk = m_neighbors[XNEG];
                            }
                            if (nextPos.z >= 16) {
                                nextPos.z = 0;
                                nextChunk = m_neighbors[ZPOS];
                            } else if (nextPos.z < 0) {
                                nextPos.z = 15;
                                nextChunk = m_neighbors[ZNEG];
                            }
                            if (nextChunk != nullptr) {
                                auto nextBlock = nextChunk->getBlockAt(nextPos.x, nextPos.y, nextPos.z);
                                canRender = !isOpaque(nextBlock) && currType != nextBlock;
                            }
                        }
                        if (canRender) {
                            for (auto &&vd : neighborFace.vertices) {
                                // Store all the per-vertex data in an interleaved format in a single VBO
                                // (except for indices, which must be stored in a separate buffer)
                                // position
                                bufUsing.push_back(glm::vec4(x, y, z, 1) + vd.pos);
                                // normal
                                bufUsing.push_back(glm::vec4(neighborFace.directionVec, 0));
                                // color
                                bufUsing.push_back(glm::vec4(uvs.at(uvs.count(currType) ? currType : ICE)[neighborFace.direction] + vd.uv,
                                                   currType == WATER || currType == LAVA ? 1 : 0, 0));
                                countUsing++;
                            }
                            auto i = countUsing - 1;
                            idxUsing.push_back(i);
                            idxUsing.push_back(i - 2);
                            idxUsing.push_back(i - 1);
                            idxUsing.push_back(i);
                            idxUsing.push_back(i - 3);
                            idxUsing.push_back(i - 2);
                        }
                    }
                }
            }
        }
    }
    // opaque
    this->m_vboData.m_vboDataOpaque = interleaved;
    this->m_vboData.m_idxDataOpaque = idx;
    // transparent
    this->m_vboData.m_vboDataTransparent = interleaved2;
    this->m_vboData.m_idxDataTransparent = idx2;
}

void Chunk::fillChunk() {
    int x = m_pos.x;
    int z = m_pos.y;
    int isGrassLand = rand()%2;
        // Populate blocks by x, z coordinates
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            glm::vec2 pos(i + x, j + z);
            glm::vec2 eleMoi = eleMoiValue(pos/128.f);
            setBlockAt(i,0,j,BEDROCK);
            for (int y = 1; y <= 95; y++) {
                if (perlinNoise(glm::vec3(x + i, y, z + j) / 10.f) > 0) {
                    setBlockAt(i, y, j, STONE);
                } else {
                    if (y < 25) {
                        setBlockAt(i, y, j, LAVA);
                    } else {
                        setBlockAt(i, y, j, EMPTY);
                    }
                }
            }
            int height = glm::mix(grasslandValue(pos), mountainValue(pos), eleMoi[0]);
            if (eleMoi[0] > 0.3) {
                for (int y = 96; y <= height; ++y) {
                    if (y <= 128) {
                        setBlockAt(i, y, j, STONE);
                    } else if (y < 200 || y < height) {
                        setBlockAt(i, y, j,
                                   random1(glm::vec2(i, y)) < 0.9 ? STONE
                                                                       : DIRT);
                    } else {
                        setBlockAt(i, y, j, SNOW);
                    }
                }
            } else {
                for (int y = 96; y < height; y++) {
                    setBlockAt(i, y, j, y <= 128 ? STONE : DIRT);
                }
                if (height > 138) {
                    if (height <= 143) {
                        setBlockAt(i, height, j, ICE);
                    } else {
                        setBlockAt(i, height, j, GRASS);
                    }
                } else {
                    for (int y = height; y <= 138; y++) {
                        setBlockAt(i, y, j, WATER);
                    }
                }
            }

        }
    }
}

void Chunk::create(std::vector<glm::vec4> m_vboDataOpaque, std::vector<GLuint> m_idxDataOpaque,
            std::vector<glm::vec4> m_vboDataTransparent, std::vector<GLuint> m_idxDataTransparent) {
    // Takes in a vector of interleaved vertex data and a vector of index data,
    // and buffers them into the appropriate VBOs of Drawable
    m_count = m_idxDataOpaque.size();

    generatePos();
    bindPos();
    mp_context->glBufferData(GL_ARRAY_BUFFER, m_vboDataOpaque.size() * sizeof(glm::vec4), m_vboDataOpaque.data(), GL_STATIC_DRAW);

    generateIdx();
    bindIdx();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_idxDataOpaque.size() * sizeof(GLuint), m_idxDataOpaque.data(), GL_STATIC_DRAW);

    // transparent
    m_count2 = m_idxDataTransparent.size();

    generatePos2();
    bindPos2();
    mp_context->glBufferData(GL_ARRAY_BUFFER, m_vboDataTransparent.size() * sizeof(glm::vec4), m_vboDataTransparent.data(), GL_STATIC_DRAW);

    generateIdx2();
    bindIdx2();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_idxDataTransparent.size() * sizeof(GLuint), m_idxDataTransparent.data(), GL_STATIC_DRAW);
}

void Chunk::setMCount(int c) {
    m_count = c;
}
