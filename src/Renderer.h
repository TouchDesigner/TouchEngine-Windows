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
#include <array>
#include <memory>

class Renderer
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
	virtual DWORD
	getWindowStyleFlags() const
	{
		return 0;
	}

	virtual const std::wstring& getDeviceName() const = 0;

	virtual bool	setup(HWND window);
	virtual bool	configure(TEInstance* instance, std::wstring& error);
	virtual bool	doesInputTextureTransfer() const;
	virtual void	resize(int width, int height);
	virtual void	stop();
	virtual bool	render() = 0;
	void			setBackgroundColor(float r, float g, float b);


	virtual size_t		getInputImageCount() const = 0;
	virtual void		beginImageLayout();
	virtual void		addInputImage(const unsigned char *rgba, size_t bytesPerRow, int width, int height);
	virtual bool		getInputImage(size_t index, TouchObject<TETexture> & texture, TouchObject<TESemaphore> & semaphore, uint64_t & waitValue) = 0;
	virtual void		clearInputImages() = 0;
	size_t				getRightSideImageCount();
	virtual void		addOutputImage();
	virtual void		endImageLayout();
						
	virtual bool		updateOutputImage(const TouchObject<TEInstance>& instance, size_t index, const std::string& identifier) = 0;
	const TouchObject<TETexture>& getOutputImage(size_t index) const;
	virtual void		clearOutputImages(); // TODO: ?
	virtual TEGraphicsContext* getTEContext() const = 0;
protected:
	bool				inputDidChange(size_t index) const;
	void				markInputChange(size_t index);
	void				markInputUnchanged(size_t index);
	void				setOutputImage(size_t index, const TouchObject<TETexture>& texture);
	std::array<float, 3>	myBackgroundColor;
	int		myWidth = 0;
	int		myHeight = 0;
private:
	HWND	myWindow = 0;
	std::vector<TouchObject<TETexture>> myOutputImages;
	std::vector<bool>			myInputImageUpdates;
};

