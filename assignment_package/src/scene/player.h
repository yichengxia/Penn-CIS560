#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"

class Player : public Entity {
private:
    glm::vec3 m_velocity, m_acceleration;
    Camera m_camera;
    glm::vec2 m_cameraOrientation;
    Terrain &mcr_terrain;
    glm::vec3 m_maxVelocity, m_minVelocity;
    bool inFlightMode, isFlyingUp;
    const glm::vec3 Acceleration;
    const glm::vec3 Friction;
    const float Gravity;
    const glm::vec3 MaxVelocity, MinVelocity;
    float FlightModeHeight;
    const float MaxFlightHeight, MinFlightHeight;
    const float FlyUpAcceleration, JumpVelocity;


    void processInputs(InputBundle &inputs);
    void computePhysics(float dT, const Terrain &terrain);

public:
    // Readonly public reference to our camera
    // for easy access from MyGL
    const Camera& mcr_camera;
    glm::vec3 mcr_posPrev;

    Player(glm::vec3 pos, Terrain &terrain);
    virtual ~Player() override;

    void setCameraWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

    // Player overrides all of Entity's movement
    // functions so that it transforms its camera
    // by the same amount as it transforms itself.
    void moveAlongVector(glm::vec3 dir) override;
    void moveAlongVectorWithCollisions(glm::vec3 dir);
    void moveForwardLocal(float amount) override;
    void moveRightLocal(float amount) override;
    void moveUpLocal(float amount) override;
    void moveForwardGlobal(float amount) override;
    void moveRightGlobal(float amount) override;
    void moveUpGlobal(float amount) override;
    void rotateOnForwardLocal(float degrees) override;
    void rotateOnRightLocal(float degrees) override;
    void rotateOnUpLocal(float degrees) override;
    void rotateOnForwardGlobal(float degrees) override;
    void rotateOnRightGlobal(float degrees) override;
    void rotateOnUpGlobal(float degrees) override;

    // For sending the Player's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;

    void changeFlightMode();
    void removeBlock();
    void placeBlock();
};

bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit, glm::ivec3 *prevCell);
