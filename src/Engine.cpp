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
#include "Engine.h"
#include "Rational.h"
#include <stdexcept>

class EngineError : public std::exception {
public:
	EngineError(TEResult result)
		: myResult(result)
	{

	}
	virtual char const* what() const override
	{
		return TEResultGetDescription(myResult);
	}
private:
	TEResult myResult;
};

static bool throwIfError(TEResult result)
{
	if (TEResultGetSeverity(result) == TESeverityError)
	{
		throw EngineError(result);
	}
	return false;
}

Engine::Engine(TEGraphicsContext *ctx, Observer& observer)
	: myObserver(observer)
{
	TEResult result = TEInstanceCreate(eventCallback, linkCallback, this, myInstance.take());
	if (result == TEResultSuccess)
	{
		result = TEInstanceAssociateGraphicsContext(myInstance, ctx);
	}
	if (result == TEResultSuccess)
	{
		// configuring without a path allows the instance to ready itself now
		result = TEInstanceConfigure(myInstance, nullptr, TETimeExternal, TEUINone);
	}
	throwIfError(result);
}

bool Engine::valid() const
{
	return myInstance.get() == nullptr;
}

void Engine::setFrameRate(const Rational& rate)
{
	TEResult result = TEInstanceSetFrameRate(myInstance, rate.numerator, rate.denominator);
	throwIfError(result);
}

Rational Engine::getFrameRate() const
{
	Rational rate;
	TEResult result = TEInstanceGetFrameRate(myInstance, &rate.numerator, &rate.denominator);
	throwIfError(result);
	return rate;
}

void Engine::open(const std::string& path)
{
	TEResult result = TEInstanceConfigure(myInstance, path.c_str(), TETimeExternal, TEUINone);
	if (result == TEResultSuccess)
	{
		result = TEInstanceLoad(myInstance);
	}
	if (result == TEResultSuccess)
	{
		result = TEInstanceResume(myInstance);
	}
	throwIfError(result);
}

void Engine::update()
{
	std::queue<Event> queue;
	// only hold the lock in this scope
	{
		std::lock_guard<std::mutex> guard(myLock);
		std::swap(queue, myEventQueue);
	}
	
	while (!queue.empty())
	{
		const auto& event = queue.front();
		switch (event.source)
		{
		case Event::Source::Instance:
			doInstanceEvent(event.instance, event.result);
			break;
		case Event::Source::Link:
			doLinkEvent(event.link, event.identifier);
			break;
		default:
			break;
		}
		queue.pop();
	}
}

void Engine::nextFrame(const Rational &time)
{
	if (myLoaded && !myInFrame)
	{
		myInFrame = true;

		TEResult result = TEInstanceStartFrameAtTime(myInstance, time.numerator, time.denominator, false);
		throwIfError(result);
	}
}

const TouchObject<TEInstance>& Engine::getInstance() const
{
	return myInstance;
}

std::string Engine::firstTextureInput() const
{
	return firstLink(TEScopeInput, [](TELinkInfo* i) { return i->type == TELinkTypeTexture; });
}

std::string Engine::firstGeometryInput() const
{
	return firstLink(TEScopeInput, [](TELinkInfo* i) { return i->type == TELinkTypeGeometry; });
}

std::string Engine::firstTextureOutput() const
{
	return firstLink(TEScopeOutput, [](TELinkInfo* i) { return i->type == TELinkTypeTexture; });
}

std::string Engine::doubleInput(const std::string& name) const
{
	return firstLink(TEScopeInput, [name](TELinkInfo* i) { return name == i->name; });
}

void Engine::setInput(const std::string& identifier, double value)
{
	TEResult result = TEInstanceLinkSetDoubleValue(myInstance, identifier.c_str(), &value, 1);
	throwIfError(result);
}

void Engine::setInput(const std::string& identifier, const TouchObject<TETexture>& texture, const TouchObject<TEGraphicsContext>& context)
{
	TEResult result = TEInstanceLinkSetTextureValue(myInstance, identifier.c_str(), texture, context);
	throwIfError(result);
}

void Engine::setInput(const std::string& identifier, const TouchObject<TEGeometry>& geometry)
{
	TEResult result = TEInstanceLinkSetGeometryValue(myInstance, identifier.c_str(), geometry);
	throwIfError(result);
}

bool Engine::outputHasChanged(const std::string& identifier)
{
	return myPendingValueChanges.contains(identifier);
}

void Engine::getOutput(const std::string& identifier, TouchObject<TETexture>& texture)
{
	myPendingValueChanges.erase(identifier);
	TEResult result = TEInstanceLinkGetTextureValue(myInstance, identifier.c_str(), TELinkValueCurrent, texture.take());
	throwIfError(result);
}

bool Engine::getResourceTransfer(TouchObject<TEObject> resource, TouchObject<TESemaphore>& semaphore, uint64_t& waitValue)
{
	if (resource)
	{
		TEResult result = TEInstanceGetResourceTransfer(myInstance, resource, semaphore.take(), &waitValue);
		if (result == TEResultSuccess)
		{
			return true;
		}
		if (result == TEResultNoMatchingEntity)
		{
			result = TEResultSuccess;
		}
		throwIfError(result);
	}
	return false;
}

bool Engine::isLoaded() const
{
	return myLoaded;
}

void Engine::end()
{
	myInstance.reset();
}

void Engine::eventCallback(TEInstance* instance, TEEvent event, TEResult result, int64_t start_time_value, int32_t start_time_scale, int64_t end_time_value, int32_t end_time_scale, void* TE_NULLABLE info)
{
	Engine* engine = reinterpret_cast<Engine*>(info);
	engine->event({ .source = Event::Source::Instance, .instance = event, .result = result});
}

void Engine::linkCallback(TEInstance* instance, TELinkEvent event, const char* identifier, void* info)
{
	Engine* engine = reinterpret_cast<Engine*>(info);
	engine->event({ .source = Event::Source::Link, .link = event, .identifier = identifier });
}

void Engine::event(Event&& event)
{
	std::lock_guard<std::mutex> guard(myLock);
	myEventQueue.push(event);
}

void Engine::doInstanceEvent(TEEvent event, TEResult result)
{
	switch (event)
	{
	case TEEventGeneral:
		throwIfError(result);
		break;
	case TEEventInstanceDidConfigure:
		if (!throwIfError(result) && result != TEResultCancelled)
		{
			myObserver.engineConfigured();
		}
		break;
	case TEEventInstanceDidLoad:
		if (!throwIfError(result) && result != TEResultCancelled)
		{
			myLoaded = true;
			myObserver.engineLoaded();
		}
		break;
	case TEEventInstanceDidUnload:
		throwIfError(result);
		break;
	case TEEventFrameDidFinish:
		myInFrame = false;
		throwIfError(result);
		break;
	default:
		break;
	}
}

void Engine::doLinkEvent(TELinkEvent event, const std::string& identifier)
{
	switch (event)
	{
	case TELinkEventAdded:
	case TELinkEventRemoved:
	case TELinkEventMoved:
	case TELinkEventChildChange:
		myPendingLayoutChange = true;
		break;
	case TELinkEventModified:
		// you may want to update eg UI in response to this
		break;
	case TELinkEventStateChange:
		// enabled/editable state changed
		break;
	case TELinkEventValueChange:
		myPendingValueChanges.insert(identifier);
		break;
	default:
		break;
	}
}

std::string Engine::firstLink(TEScope scope, const LinkMatchFunction& f) const
{
	TouchObject<TEStringArray> groups;
	TEResult result = TEInstanceGetLinkGroups(myInstance, scope, groups.take());
	throwIfError(result);
	std::string match;
	firstLink(groups, f, match);
	return match;
}

// Recursive search through links
bool Engine::firstLink(const TEStringArray* group, const LinkMatchFunction& f, std::string& match) const
{
	for (int32_t i = 0; i < group->count; i++)
	{
		TouchObject<TELinkInfo> info;
		TEResult result = TEInstanceLinkGetInfo(myInstance, group->strings[i], info.take());
		throwIfError(result);
		if (f(info))
		{
			match = info->identifier;
			return true;
		}
		else if (info->type == TELinkTypeGroup ||
			info->type == TELinkTypeSequence ||
			info->type == TELinkTypeComplex)
		{
			TouchObject<TEStringArray> children;
			result = TEInstanceLinkGetChildren(myInstance, info->identifier, children.take());
			throwIfError(result);
			if (firstLink(children, f, match))
			{
				return true;
			}
		}
	}
	return false;
}

bool Engine::isInFrame() const
{
	return myInFrame;
}
