#pragma once
#include "openglcontext.h"
#include "glm_includes.h"

// A class representing a frame buffer in the OpenGL pipeline.
// Stores three GPU handles: one to a frame buffer object, one to
// a texture object that will store the frame buffer's contents,
// and one to a depth buffer needed to properly render to the frame
// buffer.
// Redirect your render output to a FrameBuffer by invoking
// bindFrameBuffer() before ShaderProgram::draw, and read
// from the frame buffer's output texture by invoking
// bindToTextureSlot() and then associating a ShaderProgram's
// sampler2d with the appropriate texture slot.
class FrameBuffer {
private:
    OpenGLContext *mp_context;
    GLuint m_frameBuffer;
    GLuint m_outputTexture;
    GLuint m_depthRenderBuffer;

    unsigned int m_width, m_height, m_devicePixelRatio;
    bool m_created;

    unsigned int m_textureSlot;

public:
    FrameBuffer(OpenGLContext *context, unsigned int width, unsigned int height, unsigned int devicePixelRatio);
    // Make sure to call resize from MyGL::resizeGL to keep your frame buffer up to date with
    // your screen dimensions
    void resize(unsigned int width, unsigned int height, unsigned int devicePixelRatio);
    // Initialize all GPU-side data required
    void create();
    // Deallocate all GPU-side data
    void destroy();
    void bindFrameBuffer();
    // Associate our output texture with the indicated texture slot
    void bindToTextureSlot(unsigned int slot);
    unsigned int getTextureSlot() const;
};
