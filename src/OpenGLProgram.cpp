/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "stdafx.h"
#include "OpenGLProgram.h"


OpenGLProgram::OpenGLProgram()
{
}


OpenGLProgram::~OpenGLProgram()
{
}

bool
OpenGLProgram::build(const char * vs, const char * fs)
{
	destroy();

	GLuint frag = compileShader(fs, GL_FRAGMENT_SHADER);
	GLuint vert = compileShader(vs, GL_VERTEX_SHADER);

	if (frag && vert)
	{
		myProgram = glCreateProgram();
		glAttachShader(myProgram, frag);
		glAttachShader(myProgram, vert);
	}

	if (frag)
	{
		glDeleteShader(frag);
	}
	if (vert)
	{
		glDeleteShader(vert);
	}

	if (myProgram)
	{
		GLint status;

		glLinkProgram(myProgram);
		glGetProgramiv(myProgram, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			glDeleteProgram(myProgram);
			myProgram = 0;
		}
	}
	if (myProgram)
	{
		return true;
	}
	return false;
}

void
OpenGLProgram::destroy()
{
	if (myProgram != 0)
	{
		glDeleteProgram(myProgram);
		myProgram = 0;
	}
}

GLuint
OpenGLProgram::compileShader(const char * source, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		glDeleteShader(shader);
		shader = 0;
	}

	return shader;
}
