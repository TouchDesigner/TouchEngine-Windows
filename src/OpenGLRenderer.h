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

	virtual DWORD
	getWindowStyleFlags() const
	{
		return CS_OWNDC | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
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
	virtual TEGraphicsContext*
	getTEContext() const override
	{
		return myContext;
	}

	virtual bool	setup(HWND window);
	virtual void	resize(int width, int height) override;
	virtual void	stop();
	virtual bool	render();
	virtual size_t	getInputImageCount() const;
	virtual void	addInputImage(const unsigned char *rgba, size_t bytesPerRow, int width, int height);
	virtual void	clearInputImages();
	virtual void	addOutputImage();
	virtual void	setOutputImage(size_t index, const TouchObject<TETexture> &texture) override;
	virtual void	clearOutputImages();
	virtual bool	getInputImage(size_t index, TouchObject<TETexture>& texture, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue) override;
	virtual bool	releaseOutputImage(size_t index, TouchObject<TETexture>& texture, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue) override;
	virtual void	acquireOutputImage(size_t index, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue) override;
private:
	static const char* VertexShader;
	static const char* FragmentShader;

	static void		textureReleaseCallback(GLuint texture, TEObjectEvent event, void *info);
	void			drawImages(std::vector<OpenGLImage>& images, float scale, float xOffset);

	OpenGLProgram	myProgram;
	GLuint			myVAO = 0;
	GLuint			myVBO = 0;
	GLint			myVAIndex = -1;
	GLint			myTAIndex = -1;
	HGLRC			myRenderingContext = nullptr;
	HDC				myDC = nullptr;
	TouchObject<TEOpenGLContext> myContext;
	std::vector<OpenGLImage> myInputImages;
	std::vector<OpenGLImage> myOutputImages;
};

