/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 *
 * TouchEngine
 *
 * Copyright © 2021 Derivative. All rights reserved.
 *
 */

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

