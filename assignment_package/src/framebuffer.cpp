#include "framebuffer.h"
#include <iostream>

FrameBuffer::FrameBuffer(OpenGLContext *context,
                         unsigned int width, unsigned int height, unsigned int devicePixelRatio)
    : mp_context(context), m_frameBuffer(-1),
      m_outputTexture(-1), m_depthRenderBuffer(-1),
      m_width(width), m_height(height), m_devicePixelRatio(devicePixelRatio), m_created(false)
{}

void FrameBuffer::resize(unsigned int width, unsigned int height, unsigned int devicePixelRatio) {
    m_width = width;
    m_height = height;
    m_devicePixelRatio = devicePixelRatio;
}

void FrameBuffer::create() {
    // Initialize the frame buffers and render textures
    mp_context->glGenFramebuffers(1, &m_frameBuffer);
    mp_context->glGenTextures(1, &m_outputTexture);
    mp_context->glGenRenderbuffers(1, &m_depthRenderBuffer);

    mp_context->glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Bind our texture so that all functions that deal with textures will interact with this one
    mp_context->glBindTexture(GL_TEXTURE_2D, m_outputTexture);
    // Give an empty image to OpenGL ( the last "0" )
    mp_context->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width * m_devicePixelRatio, m_height * m_devicePixelRatio, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);

    // Set the render settings for the texture we've just created.
    // Essentially zero filtering on the "texture" so it appears exactly as rendered
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Clamp the colors at the edge of our texture
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Initialize our depth buffer
    mp_context->glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    mp_context->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,  m_width * m_devicePixelRatio, m_height * m_devicePixelRatio);
    mp_context->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);

    // Set m_renderedTexture as the color output of our frame buffer
    mp_context->glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_outputTexture, 0);

    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0,
    // which we previously set to m_renderedTexture
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    mp_context->glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers

    m_created = true;
    if(mp_context->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        m_created = false;
        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
        mp_context->printGLErrorLog();
    }
}

void FrameBuffer::destroy() {
    if(m_created) {
        m_created = false;
        mp_context->glDeleteFramebuffers(1, &m_frameBuffer);
        mp_context->glDeleteTextures(1, &m_outputTexture);
        mp_context->glDeleteRenderbuffers(1, &m_depthRenderBuffer);
    }
}

void FrameBuffer::bindFrameBuffer() {
    mp_context->glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
}

void FrameBuffer::bindToTextureSlot(unsigned int slot) {
    m_textureSlot = slot;
    mp_context->glActiveTexture(GL_TEXTURE0 + slot);
    mp_context->glBindTexture(GL_TEXTURE_2D, m_outputTexture);
}

unsigned int FrameBuffer::getTextureSlot() const {
    return m_textureSlot;
}
