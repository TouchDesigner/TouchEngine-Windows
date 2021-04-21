#pragma once

#include "Drawable.h"
#include "OpenGLTexture.h"
#include "GL/glew.h"

class OpenGLImage : public Drawable
{
public:
    OpenGLImage();
    ~OpenGLImage();
    bool setup(GLint vertexAttribLocation, GLint textureAttribLocation);
    void position(float x, float y);
    void scale(float scaleX, float scaleY);
    void draw();
    void update(const OpenGLTexture &texture);
    const OpenGLTexture &getTexture() const { return myTexture; }
private:
    OpenGLTexture myTexture;
    GLuint myVAO = 0;
    GLuint myVBO = 0;
    float myScaleX{ 1.0 };
    float myScaleY{ 1.0 };
    bool myDirty;
};

