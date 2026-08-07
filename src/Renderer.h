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

#include <TouchEngine/TouchEngine.h>
#include <TouchEngine/TouchObject.h>
#include <vector>
#include "Graphics.h"
#include "Geometry.h"
#include "Picture.h"
#include "Color.h"

class Renderer : public Geometry::BufferProvider, public Picture::TextureProvider
{
public:
	Renderer();
	Renderer(const Renderer &o) = delete;
	Renderer& operator=(const Renderer& o) = delete;
	virtual ~Renderer() noexcept(false);

	HWND
	getWindow() const
	{
		return myWindow;
	}

	virtual Graphics getMode() const = 0;

	virtual const std::string& getDeviceName() const = 0;

	virtual void	setup(HWND window);
	virtual bool	configure(TEInstance* instance, std::string& error);
	virtual bool	doesInputResourceTransfer() const;
	virtual void	resize(int width, int height);
	virtual void	stop();
	virtual bool	render() = 0;
	void			setBackgroundColor(const Color &color);

	virtual void		clearInputs();
	
	virtual void		setOutputImage(const TouchObject<TETexture>& texture);
	virtual bool		setOutputImage(const TouchObject<TETexture>&, const TouchObject<TESemaphore>&, uint64_t) = 0;

	const TouchObject<TETexture>& getOutputImage() const;
	virtual void		clearOutputs();
	virtual TouchObject<TEGraphicsContext> getTEContext() const = 0;

	virtual TouchObject<TEBuffer> getHostBuffer(const void* src, size_t size) override;
	virtual TouchObject<TEBuffer> getDeviceBuffer(const void* src, size_t size) override;

	void addResourceTransfers(const TouchObject<TEInstance>& instance);
protected:
	void				addResourceTransfer(const TouchObject<TEObject>& resource, const TouchObject<TESemaphore>& semaphore, uint64_t value);
	void				clearResourceTransfer(const TouchObject<TEObject>& resource);
	size_t				alignedBufferSize(size_t size) const;
	Color	myBackgroundColor;
	int		myWidth = 0;
	int		myHeight = 0;
	size_t	myMinBufferAlignment = 0;
private:
	struct Transfer {
		TouchObject<TEObject> resource;
		TouchObject<TESemaphore> semaphore;
		uint64_t value;
	};
	HWND	myWindow = 0;
	TouchObject<TETexture> myOutputImage;
	std::vector<Transfer> myPendingTransfers;
};

