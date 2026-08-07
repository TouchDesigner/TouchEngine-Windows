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
#include <unordered_set>
#include <mutex>
#include <string>
#include <queue>
#include <functional>
#include "Graphics.h"

struct Rational;

class Engine
{
public:
	class Observer {
	public:
		virtual void engineConfigured() = 0;
		virtual void engineLoaded() = 0;
	};
	Engine(TEGraphicsContext *ctx, Observer &observer);
	bool valid() const;
	void setFrameRate(const Rational& rate);
	Rational getFrameRate() const;
	void open(const std::string& path);
	void update();
	void nextFrame(const Rational &time);
	const TouchObject<TEInstance>& getInstance() const;
	std::string firstTextureInput() const;
	std::string firstGeometryInput() const;
	std::string firstTextureOutput() const;
	std::string doubleInput(const std::string& name) const;
	void setInput(const std::string& identifier, double value);
	void setInput(const std::string& identifier, const TouchObject<TETexture>& texture, const TouchObject<TEGraphicsContext>& context);
	void setInput(const std::string& identifier, const TouchObject<TEGeometry>& geometry);
	bool outputHasChanged(const std::string& identifier);
	void getOutput(const std::string &identifier, TouchObject<TETexture>& texture);
	bool getResourceTransfer(TouchObject<TEObject> resource, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue);
	bool isLoaded() const;
	bool isInFrame() const;
	void end();
private:
	struct Event {
		enum class Source {
			Instance,
			Link
		};
		Source source;
		union {
			TEEvent instance;
			TELinkEvent link;
		};
		TEResult result = TEResultSuccess;
		std::string identifier;
	};
	static void eventCallback(TEInstance* instance,
		TEEvent event,
		TEResult result,
		int64_t start_time_value,
		int32_t start_time_scale,
		int64_t end_time_value,
		int32_t end_time_scale,
		void* TE_NULLABLE info);
	static void linkCallback(TEInstance* instance,
		TELinkEvent event,
		const char* identifier,
		void* info);
	void event(Event&& event);
	void doInstanceEvent(TEEvent event, TEResult result);
	void doLinkEvent(TELinkEvent event, const std::string& identifier);

	using LinkMatchFunction = std::function<bool(TELinkInfo*)>;
	std::string firstLink(TEScope scope, const LinkMatchFunction &f) const;
	bool firstLink(const TEStringArray* group, const LinkMatchFunction& f, std::string& match) const;
	Observer& myObserver;
	std::mutex myLock;
	TouchObject<TEInstance> myInstance;
	std::queue<Event> myEventQueue;
	std::unordered_set<std::string> myPendingValueChanges;
	bool myPendingLayoutChange = false;
	bool myLoaded = false;
	bool myInError = false;
	bool myInFrame = false;
};

