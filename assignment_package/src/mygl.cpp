#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QFileDialog>
#include <QKeyEvent>
#include <QDateTime>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this),m_progPost(this),
      m_terrain(this), m_player(glm::vec3(48.f, 200.f, 48.f), m_terrain),
      m_renderedTexture(0), m_time(0),
      m_prevFrameTime(QDateTime::currentMSecsSinceEpoch()),
      m_initialTerrainLoaded(false), m_quad(this),
      m_frameBuffer(this, this->width(), this->height(), this->devicePixelRatio())
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
    m_quad.destroyVBOdata();
    m_frameBuffer.destroy();
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
    // For transparency to work properly for WATER blocks
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();
    m_quad.createVBOdata();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");
    m_progPost.create(":/glsl/post.vert.glsl", ":/glsl/post.frag.glsl");
    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));
    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    // Load images as textures into OpenGL for the world
    glGenTextures(1, &m_renderedTexture);
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    glActiveTexture(GL_TEXTURE0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    QImage img(":/textures/minecraft_textures_all.png");
    img = (img.convertToFormat(QImage::Format_RGBA8888)).mirrored();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    m_progLambert.setSampler(0);

    m_frameBuffer.create();
    m_frameBuffer.bindFrameBuffer();
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_frameBuffer.resize(this->width(), this->height(),
                         this->devicePixelRatio());
    m_frameBuffer.destroy();
    m_frameBuffer.create();
    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    qint64 currFrameTime = QDateTime::currentMSecsSinceEpoch();
    float dT = (currFrameTime - m_prevFrameTime) * 0.001f;
    // Have the player update their position and physics
    m_inputs.focused = this->hasFocus();
    m_player.tick(dT, m_inputs);
    // Check if the terrain should expand
    // This both checks to see if the player is near the border of existing
    // terrain AND checks the status of any FBMWorkers that are generating Chunks
    m_terrain.multithreadedWork(m_player.mcr_position, m_player.mcr_posPrev, dT);

    // The terrain expansion function generateTerrain(glm::vec3 pos) will be called inside update()
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
    m_prevFrameTime = currFrameTime;
    if (!m_initialTerrainLoaded) {
        m_initialTerrainLoaded = m_terrain.initialTerrainDoneLoading();
    }
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
    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progFlat.draw(m_worldAxes);
    glEnable(GL_DEPTH_TEST);
    m_frameBuffer.bindFrameBuffer();
    glViewport(0, 0, this->width() * this->devicePixelRatio(),
               this->height() * this->devicePixelRatio());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setTime(m_time++);
    m_progInstanced.setViewProjMatrix(m_player.mcr_camera.getViewProj());


    if (m_initialTerrainLoaded) {
        renderTerrain();
        glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());

        glViewport(0, 0, this->width() * this->devicePixelRatio(),
                   this->height() * this->devicePixelRatio());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_frameBuffer.bindToTextureSlot(1);
        if (m_terrain.getBlockAt(m_player.mcr_position+glm::vec3(0,1.5,0)) == WATER) {
            // 1 is for under water visual effects
            m_progPost.setUCase(1);
        } else if (m_terrain.getBlockAt(m_player.mcr_position+glm::vec3(0,1.5,0)) == LAVA) {
            // 2 for lava
            m_progPost.setUCase(2);
        } else {
            // any other number will be normal
            m_progPost.setUCase(0);
        }
        m_progPost.draw(m_quad,1);
    }

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
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    glActiveTexture(GL_TEXTURE0);
//    m_terrain.generateTerrain(m_player.mcr_position);
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
    // For height map feature
    if (e->key() == Qt::Key_H) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open grayscale/color image"),
                                                        "/",
                                                        tr("Images (*.png *.jpeg *.jpg)"));
        if (fileName.length() > 0) {
            QImage img(fileName);
            img = img.scaled(QSize(20, 20), Qt::KeepAspectRatio, Qt::FastTransformation);
            bool allGray = img.allGray();
            int w = img.width();
            int h = img.height();
            std::vector<std::vector<float>> newHeights;
            std::vector<std::vector<std::pair<float, BlockType>>> newBlocks;
            std::vector<std::pair<glm::vec3, BlockType>> colorPairs; // (RGB, NAME) pairs
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(0, 0, 0), BLACK));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(255, 255, 255), WHITE));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(255, 0, 0), RED));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(0, 255, 0), LIME));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(0, 0, 255), BLUE));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(255, 255, 0), YELLOW));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(0, 255, 255), CYAN));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(255, 0, 255), MAGENTA));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(192, 192, 192), SILVER));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(128, 128, 128), GRAY));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(128, 0, 0), MAROON));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(128, 128, 0), OLIVE));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(0, 128, 0), GREEN));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(128, 0, 128), PURPLE));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(0, 128, 128), TEAL));
            colorPairs.push_back(std::pair<glm::vec3, BlockType>(glm::vec3(0, 0, 128), NAVY));
            for (int i = 0; i < w; i++) {
                newHeights.push_back(std::vector<float>());
                newBlocks.push_back(std::vector<std::pair<float, BlockType>>());
                for (int j = 0; j < h; j++) {
                    QColor c = img.pixel(i, j);
                    if (allGray) {
                        float greyscale = c.red();
                        newHeights[i].push_back(greyscale / 255.f * 32.f + 128.f);
                    } else {
                        float greyscale = 0.2126 * c.red() + 0.7152 * c.green() + 0.0722 * c.blue();
                        BlockType t = BLACK;
                        int minDistSquare = (c.red() - colorPairs[0].first.x) * (c.red() - colorPairs[0].first.x) +
                                            (c.green() - colorPairs[0].first.y) * (c.green() - colorPairs[0].first.y) +
                                            (c.blue() - colorPairs[0].first.z) * (c.blue() - colorPairs[0].first.z);
                        for (int k = 1; k < (int) colorPairs.size(); k++) {
                            int distSquare = (c.red() - colorPairs[k].first.x) * (c.red() - colorPairs[k].first.x) +
                                             (c.green() - colorPairs[k].first.y) * (c.green() - colorPairs[k].first.y) +
                                             (c.blue() - colorPairs[k].first.z) * (c.blue() - colorPairs[k].first.z);
                            if (minDistSquare > distSquare) {
                                minDistSquare = distSquare;
                                t = colorPairs[k].second;
                            }
                        }
                        newBlocks[i].push_back(std::pair<float, BlockType>(greyscale / 255.f * 32.f + 128.f, t));
                    }
                }
            }
            if (allGray) {
                m_terrain.updateGreyscaleHeights(m_player.mcr_position.x, m_player.mcr_position.z, newHeights);
            } else {
                m_terrain.updateColorHeights(m_player.mcr_position.x, m_player.mcr_position.z, newBlocks);
            }
        }
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
