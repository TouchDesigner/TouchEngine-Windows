#include "stdafx.h"
#include "OpenGLImage.h"

OpenGLImage::OpenGLImage()
{
}


OpenGLImage::~OpenGLImage()
{
    if (myVAO)
    {
        glDeleteVertexArrays(1, &myVAO);
    }
    if (myVBO)
    {
        glDeleteBuffers(1, &myVBO);
    }
}

bool OpenGLImage::setup(GLint vertexAttribLocation, GLint textureAttribLocation)
{
    glGenVertexArrays(1, &myVAO);
    glGenBuffers(1, &myVBO);

    glBindVertexArray(myVAO);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO);

    glEnableVertexAttribArray(vertexAttribLocation);
    glVertexAttribPointer(vertexAttribLocation, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

    glEnableVertexAttribArray(textureAttribLocation);
    glVertexAttribPointer(textureAttribLocation, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(2 * sizeof(GLfloat)));

    return true;
}

void OpenGLImage::position(float x, float y)
{
    if (x != myX || y != myY)
    {
        myX = x;
        myY = y;
        myDirty = true;
    }
}

void OpenGLImage::scale(float s)
{
    if (s != myScale)
    {
        myScale = s;
        myDirty = true;
    }
}

void OpenGLImage::draw()
{
    if (myDirty)
    {
        glBindBuffer(GL_ARRAY_BUFFER, myVBO);

        GLfloat vertices[] = {
            (-1.0f * myScale) + myX,   (-1.0f * myScale) + myY,   0.0f,    myTexture.getFlipped() ? 1.0f : 0.0f,
            (-1.0f * myScale) + myX,    (1.0f * myScale) + myY,   0.0f,    myTexture.getFlipped() ? 0.0f : 1.0f,
            (1.0f * myScale) + myX,   (-1.0f * myScale) + myY,   1.0f,    myTexture.getFlipped() ? 1.0f : 0.0f,
            (1.0f * myScale) + myX,    (1.0f * myScale) + myY,   1.0f,    myTexture.getFlipped() ? 0.0f : 1.0f
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        myDirty = false;
    }

    if (myTexture.isValid())
    {
        glBindTexture(GL_TEXTURE_2D, myTexture.getName());
        glBindVertexArray(myVAO);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void OpenGLImage::update(const OpenGLTexture & texture)
{
    if (myTexture.getWidth() != texture.getWidth() || myTexture.getHeight() != texture.getHeight() || myTexture.getFlipped() != texture.getFlipped())
    {
        myDirty = true;
    }
    myTexture = texture;
}
