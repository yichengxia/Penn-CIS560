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
bool Chunk::isOpaque(BlockType t) {
    return t != EMPTY;
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
            return glm::vec4(0.5f,0.2f,0.f,1.f);
        case LAVA:
            return glm::vec4(0.9f,0.0f,0.3f,1.f);
        case BEDROCK:
            return glm::vec4(0.6f,0.5f,0.5f,1.f);
        default:
            // Other block types are not yet handled, so we default to debug purple
            return glm::vec4(1.f, 0.f, 1.f, 1.f);
    }
}

void Chunk::createVBOdata() {
    // Initialize vectors to store interleaved and indices
    std::vector<glm::vec4> interleaved;
    std::vector<GLuint> idx;

    unsigned count = 0;
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            for (int y = 0; y < 256; y++) {
                BlockType currType = getBlockAt(x, y, z);
                if (isOpaque(currType)) {
                    for (auto &&neighborFace : adjacentFaces) {
                        bool canRender = false;
                        glm::vec3 nextPos{x + neighborFace.directionVec.x, y + neighborFace.directionVec.y, z + neighborFace.directionVec.z};
                        if ((nextPos.x < 0 || nextPos.x >= 16) || (nextPos.y < 0 || nextPos.y >= 256) || (nextPos.z < 0 || nextPos.z >= 16) ||
                            !isOpaque(getBlockAt(static_cast<unsigned>(nextPos.x), static_cast<unsigned>(nextPos.y), static_cast<unsigned>(nextPos.z)))) {
                            canRender = true;
                        }
                        if (canRender) {
                            for (auto &&vd : neighborFace.vertices) {
                                // Store all the per-vertex data in an interleaved format in a single VBO
                                // (except for indices, which must be stored in a separate buffer)
                                // position
                                interleaved.push_back(glm::vec4(x, y, z, 1) + vd.pos);
                                // normal
                                interleaved.push_back(glm::vec4(neighborFace.directionVec, 0));
                                // color
                                interleaved.push_back(getColor(currType));
                                count++;
                            }
                            auto i = count - 1;
                            idx.push_back(i);
                            idx.push_back(i - 2);
                            idx.push_back(i - 1);
                            idx.push_back(i);
                            idx.push_back(i - 3);
                            idx.push_back(i - 2);
                        }
                    }
                } else {
                    // todo: transparent
                }
            }
        }
    }

    // opaque
    this->m_vboData.m_vboDataOpaque = interleaved;
    this->m_vboData.m_idxDataOpaque = idx;
    // todo: transparent
//    this->m_vboData.m_vboDataTransparent = ;
//    this->m_vboData.m_idxDataTransparent = ;
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
                    setBlockAt(i, height, j, GRASS);
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

    // todo: transparent
}

void Chunk::setMCount(int c) {
    m_count = c;
}
