#pragma once

#include <TouchEngine/TouchEngine.h>

/*
* Utility for TouchObject
*/
template <typename T, typename U, typename = void>
struct TouchIsMemberOf : std::false_type
{};

template <typename T, typename U>
struct TouchIsMemberOf<T, U, typename std::enable_if_t<
	std::is_same<T, TEObject>::value ||
	(std::is_same<T, TETexture>::value && (
		std::is_same<U, TEOpenGLTexture>::value ||
		std::is_same<U, TEDXGITexture>::value ||
		std::is_same<U, TED3D11Texture>::value)) ||
	(std::is_same<T, TEGraphicsContext>::value && (
		std::is_same<U, TEOpenGLContext>::value ||
		std::is_same<U, TED3D11Context>::value))>> : std::true_type
{};

/*
* Wrapper for TEObjects
*/
template <typename T>
class TouchObject
{
public:
	TouchObject()
	{	};
	
	TouchObject(T* o)
		: myObject(static_cast<T *>(TERetain(o)))
	{	};

	~TouchObject()
	{
		TERelease(&myObject);
	}
	
	TouchObject(const TouchObject<T>& o)
		: myObject(static_cast<T *>(TERetain(o.myObject)))
	{	}
	
	TouchObject(TouchObject<T>&& o)
		: myObject(o.myObject)
	{
		o.myObject = nullptr;
	}
	
	template <typename O, std::enable_if_t<TouchIsMemberOf<T, O>::value, int > = 0 >
	TouchObject(const TouchObject<O>& o)
		: TouchObject(o.get())
	{	};
	
	TouchObject& operator=(const TouchObject<T>& o)
	{
		if (&o != this)
		{
			TERetain(o.myObject);
			TERelease(&myObject);
			myObject = o.myObject;
		}
		return *this;
	}
	
	TouchObject<T>& operator=(TouchObject<T>&& o)
	{
		TERelease(&myObject);
		myObject = o.myObject;
		o.myObject = nullptr;
		return *this;
	}
	
	template <typename O, std::enable_if_t<TouchIsMemberOf<T, O>::value, int > = 0 >
	TouchObject& operator=(const TouchObject<O>& o)
	{
		set(o.get());
		return *this;
	}
	
	void reset()
	{
		TERelease(&myObject);
	}
	
	operator T*() const
	{
		return myObject;
	}
	
	T* get() const
	{
		return myObject;
	}
	
	void set(T* o)
	{
		TERetain(o);
		TERelease(&myObject);
		myObject = o;
	}
	
	/*
	* Use take() to pass to functions which return a TEObject through a parameter
	* This TouchObject takes ownership of the TEObject when it is returned.
	*/
	T** take()
	{
		TERelease(&myObject);
		return &myObject;
	}
private:
	T* myObject{ nullptr };

};

