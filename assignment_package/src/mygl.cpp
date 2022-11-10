#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this),
      m_terrain(this), m_player(glm::vec3(48.f, 188.f, 48.f), m_terrain),
      m_prevFrameTime(QDateTime::currentMSecsSinceEpoch())
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    // We do not render the faces inside the terrain
    glEnable(GL_CULL_FACE);

    m_terrain.CreateTestScene();
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    qint64 currFrameTime = QDateTime::currentMSecsSinceEpoch();
    float dT = (currFrameTime - m_prevFrameTime) * 0.1f;
    m_inputs.focused = this->hasFocus();
    m_player.tick(dT, m_inputs);
    // The terrain expansion function generateTerrain(glm::vec3 pos) will be called inside update()
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
    m_prevFrameTime = currFrameTime;
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progInstanced.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    renderTerrain();

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progFlat.draw(m_worldAxes);
    glEnable(GL_DEPTH_TEST);
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    m_terrain.generateTerrain(m_player.mcr_position);
    auto chunkX = glm::floor(m_player.mcr_position.x / 16.f) * 16, chunkZ = glm::floor(m_player.mcr_position.z / 16.f) * 16;
    m_terrain.draw(chunkX - 64, chunkX + 65, chunkZ - 64, chunkZ + 65, &m_progLambert);
}


void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_inputs.mouseX = -100;
    } else if (e->key() == Qt::Key_Left) {
        m_inputs.mouseX = 100;
    } else if (e->key() == Qt::Key_Up) {
        m_inputs.mouseY = 100;
    } else if (e->key() == Qt::Key_Down) {
        m_inputs.mouseY = -100;
    }
    if (e->key() == Qt::Key_W) {
        //  In flight mode: Accelerate positively along forward vector
        //  In Ground mode: Accelerate positively along forward vector, discarding Y component and re-normalizing
        m_inputs.wPressed = true;
    }
    if (e->key() == Qt::Key_S) {
        // In flight mode: Accelerate negatively along forward vector
        // In Ground mode: Accelerate negatively along forward vector, discarding Y component and re-normalizing
        m_inputs.sPressed = true;
    }
    if (e->key() == Qt::Key_D) {
        // In flight mode: Accelerate positively along right vector
        // In Ground mode: Accelerate positively along right vector, discarding Y component and re-normalizing
        m_inputs.dPressed = true;
    }
    if (e->key() == Qt::Key_A) {
        // In flight mode: Accelerate negatively along right vector
        // In Ground mode: Accelerate negatively along right vector, discarding Y component and re-normalizing
        m_inputs.aPressed = true;
    }
    if (e->key() == Qt::Key_Q) {
        // In flight mode: Accelerate negatively along up vector
        m_inputs.qPressed = true;
    }
    if (e->key() == Qt::Key_E) {
        // In flight mode: Accelerate positively along up vector
        m_inputs.ePressed = true;
    }
    if (e->key() == Qt::Key_F) {
        // In flight mode: Toggle flight mode OFF
        // In Ground mode: Toggle flight mode ON
        m_player.changeFlightMode();
    }
    if (e->key() == Qt::Key_Space) {
        // In Ground mode: Add a vertical component to the player's velocity to make them jump
        m_inputs.spacePressed = true;
   }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Right) {
        m_inputs.mouseX = 0;
    } else if (e->key() == Qt::Key_Left) {
        m_inputs.mouseX = 0;
    } else if (e->key() == Qt::Key_Up) {
        m_inputs.mouseY = 0;
    } else if (e->key() == Qt::Key_Down) {
        m_inputs.mouseY = 0;
    }
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    }
    if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    }
    if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    }
    if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    }
    if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    }
    if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    }
    if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
   }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    // MS1.3
    QPoint mousePos = e->pos();
    QPoint centerPos = QPoint(width() / 2, height() / 2);
    if (this->hasFocus()){
        m_inputs.mouseX = centerPos.x() - mousePos.x();
        m_inputs.mouseY = centerPos.y() - mousePos.y();
        moveMouseToCenter();
    } else {
        m_inputs.mouseX = 0;
        m_inputs.mouseY = 0;
    }

}

void MyGL::mousePressEvent(QMouseEvent *e) {
    // MS1.3
    if (e->button() == Qt::LeftButton) {
        m_player.removeBlock();
    } else if (e->button() == Qt::RightButton) {
        m_player.placeBlock();
    }
}
