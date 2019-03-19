#pragma once

#include "OpenGLTexture.h"
#include "GL/glew.h"

class OpenGLImage
{
public:
    OpenGLImage();
    ~OpenGLImage();
    bool setup(GLint vertexAttribLocation, GLint textureAttribLocation);
    void position(float x, float y);
    void scale(float s);
    void draw();
    void update(const OpenGLTexture &texture);
    const OpenGLTexture &getTexture() const { return myTexture; }
private:
    OpenGLTexture myTexture;
    GLuint myVAO = 0;
    GLuint myVBO = 0;
    float myX;
    float myY;
    float myScale;
    bool myDirty;
};

