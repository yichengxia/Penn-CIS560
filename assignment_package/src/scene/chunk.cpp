#include "chunk.h"


Chunk::Chunk(OpenGLContext* mp_context) : Drawable(mp_context), m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    if ((int) x < 0) {
        if (m_neighbors.at(XNEG) == nullptr) {
            return DEBUG;
        }
        return m_neighbors.at(XNEG)->getBlockAt(x + 16, y, z);
    } else if ((int) x >= 16) {
        if (m_neighbors.at(XPOS) == nullptr) {
            return DEBUG;
        }
        return m_neighbors.at(XPOS)->getBlockAt(x - 16, y, z);
    } else if ((int) y < 0) {
        if (m_neighbors.at(YNEG) == nullptr) {
            return DEBUG;
        }
        return m_neighbors.at(YNEG)->getBlockAt(x, y + 256, z);
    } else if ((int) y >= 256) {
        if (m_neighbors.at(YPOS) == nullptr) {
            return DEBUG;
        }
        return m_neighbors.at(YPOS)->getBlockAt(x, y - 256, z);
    } else if ((int) z < 0) {
        if (m_neighbors.at(ZNEG) == nullptr) {
            return DEBUG;
        }
        return m_neighbors.at(ZNEG)->getBlockAt(x, y, z + 16);
    } else if ((int) z >= 16) {
        if (m_neighbors.at(ZPOS) == nullptr) {
            return DEBUG;
        }
        return m_neighbors.at(ZPOS)->getBlockAt(x, y, z - 16);
    }
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Get Block at a certain position vector
BlockType Chunk::getBlockAt(glm::vec3 pos) const {
    return getBlockAt(static_cast<unsigned int>(pos.x), static_cast<unsigned int>(pos.y), static_cast<unsigned int>(pos.z));
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

void Chunk::createVBOdata() {
    // Initialize vectors to store pos, nor, col, and idx
    std::vector<glm::vec4> pos = std::vector<glm::vec4>();
    std::vector<glm::vec4> nor = std::vector<glm::vec4>();
    std::vector<glm::vec4> col = std::vector<glm::vec4>();
    std::vector<int> idx = std::vector<int>();

    int offset = 0;
    for (int z = 0; z < 16; z++) {
        for (int y = 0; y < 256; y++) {
            for (int x = 0; x < 16; x++) {
                BlockType currType = getBlockAt(x, y, z);
                if (isOpaque(currType)) {
                    glm::vec3 currPos = glm::vec3(x, y, z);
                    glm::vec3 currWorldPos = glm::vec3(x, y, z);
                    for (auto &neighborFace : adjacentFaces) {
                        BlockType neighborType = getBlockAt(neighborFace.directionVec + currPos);
                        if (!isOpaque(neighborType)) {
                            for (auto &vd : neighborFace.vertices) {
                                pos.push_back(glm::vec4(currWorldPos, 0.f) + vd.pos);
                                nor.push_back(glm::vec4(neighborFace.directionVec, 0.f));
                                switch (currType) {
                                    case GRASS:
                                        col.push_back(glm::vec4(95.f, 159.f, 53.f, 0.f) / 255.f);
                                        break;
                                    case DIRT:
                                        col.push_back(glm::vec4(121.f, 85.f, 58.f, 0.f) / 255.f);
                                        break;
                                    case STONE:
                                        col.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 0.f));
                                        break;
                                    case WATER:
                                        col.push_back(glm::vec4(0.f, 0.f, 0.75f, 0.f));
                                        break;
                                    default:
                                        // Support other block types (not yet handled); debug in purple
                                        col.push_back(glm::vec4(1.f, 0.f, 1.f, 0.f));
                                        break;
                                }
                                idx.push_back(0 + offset);
                                idx.push_back(1 + offset);
                                idx.push_back(2 + offset);
                                idx.push_back(0 + offset);
                                idx.push_back(2 + offset);
                                idx.push_back(3 + offset);
                                offset += 4;
                            }
                        }
                    }
                }
            }
        }
    }

    // Store all the per-vertex data in an interleaved format in a single VBO
    // (except for indices, which must be stored in a separate buffer)
    std::vector<glm::vec4> interleaved = std::vector<glm::vec4>();
    for (int i = 0; i < (int) pos.size(); i++) {
        interleaved.push_back(pos[i]);
        interleaved.push_back(nor[i]);
        interleaved.push_back(col[i]);
    }
    bufferVBOdata(interleaved, idx);
}

// Helper function that takes in a vector of interleaved vertex data and a vector of index data,
// and buffers them into the appropriate VBOs of Drawable
void Chunk::bufferVBOdata(std::vector<glm::vec4> interleaved, std::vector<int> idx) {
    m_count = idx.size();
    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(glm::vec4), interleaved.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(glm::vec4), interleaved.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(glm::vec4), interleaved.data(), GL_STATIC_DRAW);

}
