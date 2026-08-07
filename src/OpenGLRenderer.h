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
#include "Renderer.h"
#include <mutex>
#include <vector>
#include "OpenGLImage.h"
#include "OpenGLProgram.h"
#include "GL/glew.h"

class OpenGLRenderer :
	public Renderer
{
public:
	OpenGLRenderer();
	virtual ~OpenGLRenderer();

	virtual Graphics getMode() const override
	{
		return Graphics::OpenGL;
	}

	HDC
	getDC() const
	{
		return myDC;
	}
	HGLRC
	getRC() const
	{
		return myRenderingContext;
	}
	virtual TouchObject<TEGraphicsContext>
	getTEContext() const override
	{
		return myContext;
	}

	virtual void	setup(HWND window) override;
	virtual bool	configure(TEInstance* instance, std::string& error) override;
	virtual void	resize(int width, int height) override;
	virtual void	stop() override;
	virtual bool	render() override;
	virtual TouchObject<TETexture>	getTexture(const unsigned char* rgba, size_t bytesPerRow, int width, int height) override;
	virtual void	clearInputs() override;
	virtual bool	setOutputImage(const TouchObject<TETexture>& texture, const TouchObject<TESemaphore>& semaphore, uint64_t waitValue) override;
	virtual void	setOutputImage(const TouchObject<TETexture>& texture) override;
	virtual void	clearOutputs() override;

	virtual const std::string& getDeviceName() const override;
private:
	static const char* VertexShader;
	static const char* FragmentShader;

	static void		textureReleaseCallback(GLuint texture, TEObjectEvent event, void *info);

	OpenGLProgram	myProgram;
	GLuint			myVAO = 0;
	GLuint			myVBO = 0;
	GLint			myVAIndex = -1;
	GLint			myTAIndex = -1;
	HGLRC			myRenderingContext = nullptr;
	HDC				myDC = nullptr;
	TouchObject<TEOpenGLContext> myContext;
	OpenGLTexture	myInputTexture;
	OpenGLImage		myOutputImage;
	std::string		myDeviceName;
};

