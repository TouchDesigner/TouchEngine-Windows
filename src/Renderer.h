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

#include <TouchEngine/TouchEngine.h>
#include <TouchEngine/TouchObject.h>
#include <vector>
#include <memory>

class Renderer
{
public:
	Renderer();
	virtual ~Renderer();
	Renderer(const Renderer &o) = delete;
	Renderer& operator=(const Renderer& o) = delete;
	HWND getWindow() const { return myWindow; }
	virtual DWORD getWindowStyleFlags() const { return 0; }
	virtual bool setup(HWND window);
	virtual void resize(int width, int height);
	virtual void stop();
	virtual bool render() = 0;
	void setBackgroundColor(float r, float g, float b);
	virtual size_t getLeftSideImageCount() const = 0;
	virtual void addLeftSideImage(const unsigned char *rgba, size_t bytesPerRow, int width, int height) = 0;
	virtual TETexture *createLeftSideImage(size_t index) = 0;
	virtual void clearLeftSideImages() = 0;
	size_t getRightSideImageCount();
	virtual void addRightSideImage();
	virtual void setRightSideImage(size_t index, const TouchObject<TETexture> &texture);
	virtual void clearRightSideImages();
	virtual TEGraphicsContext* getTEContext() const = 0;
protected:
	float myBackgroundColor[3];
	int myWidth{ 0 };
	int myHeight{ 0 };
private:
	HWND myWindow;
	std::vector<TouchObject<TETexture>> myRightSideImages;
};

