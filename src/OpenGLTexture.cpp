#include "stdafx.h"
#include "OpenGLTexture.h"

OpenGLTexture::OpenGLTexture()
{
}

OpenGLTexture::OpenGLTexture(const unsigned char * rgba, size_t bytesPerRow, GLsizei width, GLsizei height)
	: myWidth(width), myHeight(height)
{
	if (bytesPerRow != width * 4)
	{
		throw "Needs work";
	}
    GLuint name;
	glGenTextures(1, &name);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
    myName = std::shared_ptr<GLuint>(new GLuint(name), [](auto p) {
        glDeleteTextures(1, p);
        delete p;
    });
}

OpenGLTexture::OpenGLTexture(GLuint name, GLsizei width, GLsizei height, std::function<void()> releaseCallback)
    : myWidth(width), myHeight(height)
{
    myName = std::shared_ptr<GLuint>(new GLuint(name), [releaseCallback](auto p) {
        releaseCallback();
        delete p;
    });
}


OpenGLTexture::~OpenGLTexture()
{
}

GLuint OpenGLTexture::getName() const
{
	return *myName;
}

GLsizei OpenGLTexture::getWidth() const
{
	return myWidth;
}

GLsizei OpenGLTexture::getHeight() const
{
	return myHeight;
}

bool OpenGLTexture::isValid() const
{
    if (myName)
    {
        return true;
    }
    return false;
}
