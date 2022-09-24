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

#pragma once
#include "GL/glew.h"
#include <memory>
#include <functional>
#include <TouchEngine/TouchObject.h>

class OpenGLTexture
{
public:
	OpenGLTexture();
	OpenGLTexture(const unsigned char *rgba, size_t bytesPerRow, GLsizei width, GLsizei height);
	OpenGLTexture(const TouchObject<TEOpenGLTexture> &texture);

	GLuint	getName() const;
	GLsizei getWidth() const;
	GLsizei getHeight() const;
	bool	getFlipped() const;
	bool	isValid() const;
	constexpr const TouchObject<TEOpenGLTexture> &
		getSource() const
	{
		return mySource;
	}
private:
	TouchObject<TEOpenGLTexture> mySource;
	std::shared_ptr<GLuint> myName;
	GLsizei		myWidth;
	GLsizei		myHeight;
	bool		myFlipped;
};

