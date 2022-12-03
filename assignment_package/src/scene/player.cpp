#include "player.h"
#include <QString>

Player::Player(glm::vec3 pos, Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5, 0)),
      m_cameraOrientation(glm::vec2(0, 0)),
      mcr_terrain(terrain),
      m_maxVelocity(glm::vec3(15)), m_minVelocity(glm::vec3(-15)),
      inFlightMode(true), isFlyingUp(false),
      Acceleration(glm::vec3(20)),
      Friction(glm::vec3(-10)),
      Gravity(-30),
      MaxVelocity(glm::vec3(15)), MinVelocity(glm::vec3(-15)),
      FlightModeHeight(139.f), MaxFlightHeight(255.f), MinFlightHeight(0.f),
      FlyUpAcceleration(20), JumpVelocity(10),
      effect(new QSoundEffect),
      mcr_camera(m_camera), mcr_posPrev(pos)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    mcr_posPrev = mcr_position;
    processInputs(input);
    computePhysics(dT, mcr_terrain);
}


float clamp(float n, float lower, float upper) {
  return std::max(lower, std::min(n, upper));
}

void Player::processInputs(InputBundle &inputs) {
    // MS1.3: Update the Player's velocity and acceleration based on the
    // state of the inputs.
    // the state of the keyboard is read and used to set the Player's physics attributes
    // and use mouse movement to rotate the camera

    // Read input from the user
    glm::vec3 inputDirection = glm::vec3(0);
    if (inputs.wPressed) {
        inputDirection += m_forward;
    }
    if (inputs.sPressed) {
        inputDirection -= m_forward;
    }
    if (inputs.dPressed) {
        inputDirection += m_right;
    }
    if (inputs.aPressed) {
        inputDirection -= m_right;
    }
    if (inFlightMode) {
        if (inputs.qPressed) {
            inputDirection -= m_up;
        }
        if (inputs.ePressed) {
            inputDirection += m_up;
        }
        if (glm::length(inputDirection) > 0) {
            inputDirection = glm::normalize(inputDirection);
        }
    } else { // in ground mode
        // Add move sound for four directions if pressed
        if (inputs.wPressed || inputs.sPressed || inputs.dPressed || inputs.aPressed) {
            effect->setSource(QUrl::fromLocalFile(":/sounds/footsteps.wav"));
            effect->setLoopCount(1);
            effect->setVolume(0.25f);
            effect->play();
        }
        // discarding Y component and re-normalizing
        inputDirection.y = 0;
        if (glm::length(inputDirection) > 0) {
            inputDirection = glm::normalize(inputDirection);
        }

        if (inputs.spacePressed) {
            //  Add a vertical component to the player's velocity to make them jump
            m_velocity.y += JumpVelocity;
        }
    }

    m_acceleration = inputDirection * Acceleration;
    for (int i = 0; i < 3; i++) {
        if (m_velocity[i] > 0) {
            m_minVelocity[i] = 0;
            m_maxVelocity[i] = MaxVelocity[i];
        } else if (m_velocity[i] < 0) {
            m_minVelocity[i] = MinVelocity[i];
            m_maxVelocity[i] = 0;
        } else {
            m_minVelocity[i] = MinVelocity[i];
            m_maxVelocity[i] = MaxVelocity[i];
        }
    }
    if (not inFlightMode) { // subject to gravity in ground mode
        m_acceleration.y += Gravity;
        m_minVelocity.y = MinVelocity.y;
        m_maxVelocity.y = MaxVelocity.y;
    } else {
        if (isFlyingUp) { // in the process of from ground to height of flightMode
            m_acceleration.y += FlyUpAcceleration;
        }
    }
    // Have player face correct direction
    if ((not inputs.focused) || (glm::abs(inputs.mouseX) < 5 && glm::abs(inputs.mouseY) < 5)) {
        return;
    }

    glm::vec2 cameraOrientationOrigin = m_cameraOrientation;

    float thetaChange = 0.05 * (inputs.mouseX);
    m_cameraOrientation.x = std::fmod(m_cameraOrientation.x + thetaChange, 360);
    float phiChange = 0.05 * (inputs.mouseY);
    m_cameraOrientation.y = clamp(m_cameraOrientation.y + phiChange, -89.9999, 89.9999);
    phiChange = m_cameraOrientation.y - cameraOrientationOrigin.y;

    rotateOnUpGlobal(thetaChange);
    rotateOnRightLocal(phiChange);
}

void Player::computePhysics(float dT, const Terrain &terrain) {
    // MS1.3: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.
    /// Computes accel and velocity
    m_velocity += glm::sign(m_velocity) * Friction * dT; // simulates friction + drag
    m_velocity += m_acceleration * dT;
    m_velocity = glm::clamp(m_velocity, m_minVelocity, m_maxVelocity);
    // Update position without moving inside the world’s geometry
    glm::vec3 amount = m_velocity * dT;
    if (inFlightMode) {
        if (isFlyingUp && (m_position.y + amount.y) >= FlightModeHeight) {
            isFlyingUp = false;
            m_velocity.y = 0;
            amount.y = fmin(amount.y, FlightModeHeight - m_position.y);
        } else {
            amount.y = fmax(fmin(amount.y, MaxFlightHeight - m_position.y), MinFlightHeight - m_position.y);
        }
        moveAlongVector(amount);
    } else { // subject to terrain collisions
        moveAlongVectorWithCollisions(amount);
    }
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}

void Player::moveAlongVectorWithCollisions(glm::vec3 dir) {
    std::array<glm::vec3, 12> rayOrigins = {
        m_position + glm::vec3(0.5, 0, 0.5),
        m_position + glm::vec3(0.5, 0, -0.5),
        m_position + glm::vec3(-0.5, 0, -0.5),
        m_position + glm::vec3(-0.5, 0, 0.5),
        m_position + glm::vec3(0.5, 1, 0.5),
        m_position + glm::vec3(0.5, 1, -0.5),
        m_position + glm::vec3(-0.5, 1, -0.5),
        m_position + glm::vec3(-0.5, 1, 0.5),
        m_position + glm::vec3(0.5, 2, 0.5),
        m_position + glm::vec3(0.5, 2, -0.5),
        m_position + glm::vec3(-0.5, 2, -0.5),
        m_position + glm::vec3(-0.5, 2, 0.5)
    };

    for (glm::vec3 rayOrigin : rayOrigins) {
        for (int axis = 0; axis < 3; axis++) {
            glm::vec3 rayDirection = glm::vec3(0);
            rayDirection[axis] = dir[axis];
            float outdist;
            glm::ivec3 out_blockHit, prevCell;
            bool isBlocked = gridMarch(rayOrigin, rayDirection, mcr_terrain, &outdist, &out_blockHit, &prevCell);
            if (isBlocked) {
                if (outdist > 0.001f) {
                    dir[axis] = glm::sign(dir[axis]) * (std::fmax(glm::min(glm::abs(dir[axis]), outdist) - 0.0001f, 0));
                } else {
                    dir[axis] = 0;
                }
                m_velocity[axis] = 0;
            }
        }
    }
    moveAlongVector(dir);
}

void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}

void Player::changeFlightMode() {
    inFlightMode = not inFlightMode;
    if (inFlightMode) { // from ground to flight
        isFlyingUp = true;
        FlightModeHeight = m_position.y + 10;
    }
}

void Player::removeBlock() {
    float outdist;
    glm::ivec3 out_blockHit, prevCell;
    bool isBlocked = gridMarch(m_camera.mcr_position, 3.f * m_forward, mcr_terrain, &outdist, &out_blockHit, &prevCell);
    if (isBlocked) {
        mcr_terrain.setBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z, EMPTY);
        const uPtr<Chunk> &c = mcr_terrain.getChunkAt(out_blockHit.x, out_blockHit.z);
        c->destroyVBOdata();
        c->createVBOdata();
        c->create(c->m_vboData.m_vboDataOpaque, c->m_vboData.m_idxDataOpaque,
                  c->m_vboData.m_vboDataTransparent, c->m_vboData.m_idxDataTransparent);
    }
}

void Player::placeBlock() {
    float outdist;
    glm::ivec3 out_blockHit, prevCell;
    bool isBlocked = gridMarch(m_camera.mcr_position, 3.f * m_forward, mcr_terrain, &outdist, &out_blockHit, &prevCell);
    if (isBlocked) {
        mcr_terrain.setBlockAt(prevCell.x, prevCell.y, prevCell.z, STONE);
        const uPtr<Chunk> &c = mcr_terrain.getChunkAt(prevCell.x, prevCell.z);
        c->destroyVBOdata();
        c->createVBOdata();
        c->create(c->m_vboData.m_vboDataOpaque, c->m_vboData.m_idxDataOpaque,
                  c->m_vboData.m_vboDataTransparent, c->m_vboData.m_idxDataTransparent);
    }
}

// prevCell is the cell adjacent to the out_blockHit
bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit, glm::ivec3 *prevCell) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    *prevCell = currCell;
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        *prevCell = currCell;
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY && cellType != WATER && cellType != LAVA) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}

