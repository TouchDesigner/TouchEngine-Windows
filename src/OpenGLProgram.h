#pragma once

#include "GL/glew.h"

class OpenGLProgram
{
public:
    OpenGLProgram();
    ~OpenGLProgram();
    bool build(const char *vs, const char *fs);
    void destroy();
    GLuint getName() const { return myProgram; }
private:
    static const char *CompileError;
    static const char *LinkError;
    static GLuint compileShader(const char *source, GLenum type);
    GLuint myProgram;
};

