/*
 * A pointer adapted from yasper, original license follows:
 *
 * yasper - A non-intrusive reference counted pointer.
 *	    Version: 1.04
 *
 *  Many ideas borrowed from Yonat Sharon and
 *  Andrei Alexandrescu.
 *
 * (zlib license)
 * ----------------------------------------------------------------------------------
 * Copyright (C) 2005-2007 Alex Rubinsteyn
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * -----------------------------------------------------------------------------------
 *
 * Send all questions, comments and bug reports to:
 * Alex Rubinsteyn (alex.rubinsteyn {at-nospam} gmail {dot} com)
 */

#ifndef yasper_ptr_h
#define yasper_ptr_h

#include <exception>

struct NullPointerException: public std::exception {
	NullPointerException() throw () {
	}
	~NullPointerException() throw () {
	}

	const char* what() const throw () {
		return "Attempted to dereference null smartptr";
	}
};

typedef void (*delete_function_ptr)(void* ptr);

/* A smart pointer adapted from yasper that allows for definition on incomplete types */
template<typename X>
class smartptr {
private:
	// Keep function around for deletion purposes so that we can work on incomplete types
	template<typename T>
	static void delete_function(void* ptr) {
		delete (T*)ptr;
	}

	struct RefHandler {
		RefHandler(delete_function_ptr deleter) :
				deleter(deleter), count(1) {
		}
		delete_function_ptr deleter;
		unsigned count;

	};

public:
	typedef X element_type;

	/*
	 ptr needs to be its own friend so ptr< X > and ptr< Y > can access
	 each other's private data members
	 */
	template<class Y> friend class smartptr;
	/*
	 default constructor
	 - don't create Counter
	 */
	smartptr() :
			rawPtr(0), refhandler(0) {
	}

	/*
	 Construct from a raw pointer
	 */
	smartptr(X* raw, RefHandler* c = 0) :
			rawPtr(0), refhandler(0) {
		if (raw) {
			rawPtr = raw;
			if (c)
				acquire(c);
			else
				refhandler = new RefHandler(delete_function<X>);
		}
	}

	template<typename Y>
	explicit smartptr(Y* raw, RefHandler* c = 0) :
			rawPtr(0), refhandler(0) {
		if (raw) {
			rawPtr = static_cast<X*>(raw);
			if (c)
				acquire(c);
			else
				refhandler = new RefHandler(delete_function<X>);
		}
	}

	template<typename Y>
	explicit smartptr(Y* raw, delete_function_ptr destructor) :
			rawPtr(0), refhandler(0) {
		if (raw) {
			rawPtr = static_cast<X*>(raw);
			refhandler = new RefHandler(destructor);
		}
	}

	/*
	 Copy constructor
	 */
	smartptr(const smartptr<X>& otherPtr) {
		acquire(otherPtr.refhandler);
		rawPtr = otherPtr.rawPtr;
	}

	template<typename Y>
	explicit smartptr(const smartptr<Y>& otherPtr) :
			rawPtr(0), refhandler(0) {
		acquire(otherPtr.counter);
		rawPtr = static_cast<X*>(otherPtr.get());
	}

	/*
	 Destructor
	 */
	~smartptr() {
		release();
	}

	/*
	 Assignment to another ptr
	 */

	smartptr& operator=(const smartptr<X>& otherPtr) {
		if (this != &otherPtr) {
			release();
			acquire(otherPtr.refhandler);
			rawPtr = otherPtr.rawPtr;
		}
		return *this;
	}

	template<typename Y>
	smartptr& operator=(const smartptr<Y>& otherPtr) {
		if (this != (smartptr<X>*)&otherPtr) {
			release();
			acquire(otherPtr.counter);
			rawPtr = static_cast<X*>(otherPtr.get());
		}
		return *this;
	}

	/*
	 Assignment to raw pointers is really dangerous business.
	 If the raw pointer is also being used elsewhere,
	 we might prematurely delete it, causing much pain.
	 Use sparingly/with caution.
	 */

	smartptr& operator=(X* raw) {

		if (raw) {
			release();
			refhandler = new RefHandler(delete_function<X>);
			rawPtr = raw;
		}
		return *this;
	}

	template<typename Y>
	smartptr& operator=(Y* raw) {
		if (raw) {
			release();
			refhandler = new RefHandler(delete_function<X>);
			rawPtr = static_cast<X*>(raw);
		}
		return *this;
	}

	/*
	 assignment to long to allow ptr< X > = NULL,
	 also allows raw pointer assignment by conversion.
	 Raw pointer assignment is really dangerous!
	 If the raw pointer is being used elsewhere,
	 it will get deleted prematurely.
	 */
	smartptr& operator=(long num) {
		if (num == 0) //pointer set to null
				{
			release();
		}

		else //assign raw pointer by conversion
		{
			release();
			refhandler = new RefHandler(delete_function<X>);
			rawPtr = reinterpret_cast<X*>(num);
		}

		return *this;
	}

	/*
	 Member Access
	 */
	X* operator->() const {
		return get();
	}

	/*
	 Dereference the pointer
	 */
	X& operator*() const {
		return *get();
	}

	/*
	 Conversion/casting operators
	 */

	operator bool() const {
		return is_valid();
	}

	template<typename Y>
	operator smartptr<Y>() {
		//new ptr must also take our counter or else the reference counts
		//will go out of sync
		return smartptr<Y>(rawPtr, refhandler);
	}

	/*
	 Provide access to the raw pointer
	 */

	X* get() const {
		if (rawPtr == 0)
			throw new NullPointerException;
		return rawPtr;
	}

	/*
	 Is there only one reference on the counter?
	 */
	bool is_unique() const {
		if (refhandler && refhandler->count == 1)
			return true;
		return false;
	}

	bool is_valid() const {
		if (refhandler && rawPtr)
			return true;
		return false;
	}

	unsigned get_count() const {
		if (refhandler)
			return refhandler->count;
		return 0;
	}

private:
	X* rawPtr;
	RefHandler* refhandler;

	// increment the count
	void acquire(RefHandler* c) {
		refhandler = c;
		if (c) {
			(c->count)++;
		}
	}

	// decrement the count, delete if it is 0
	void release() {
		if (refhandler) {
			(refhandler->count)--;

			if (refhandler->count == 0) {
				refhandler->deleter(rawPtr);
				delete refhandler;
			}
		}
		refhandler = 0;
		rawPtr = 0;

	}
};

template<typename X, typename Y>
bool operator==(const smartptr<X>& lptr, const smartptr<Y>& rptr) {
	return lptr.get() == rptr.get();
}

template<typename X, typename Y>
bool operator==(const smartptr<X>& lptr, Y* raw) {
	return lptr.get() == raw;
}

template<typename X>
bool operator==(const smartptr<X>& lptr, long num) {
	if (num == 0 && !lptr.IsValid()) //both pointer and address are null
			{
		return true;
	}

	else //convert num to a pointer, compare addresses
	{
		return lptr == reinterpret_cast<X*>(num);
	}

}

template<typename X, typename Y>
bool operator!=(const smartptr<X>& lptr, const smartptr<Y>& rptr) {
	return (!operator==(lptr, rptr));
}

template<typename X, typename Y>
bool operator!=(const smartptr<X>& lptr, Y* raw) {
	return (!operator==(lptr, raw));
}

template<typename X>
bool operator!=(const smartptr<X>& lptr, long num) {
	return (!operator==(lptr, num));
}

template<typename X, typename Y>
bool operator&&(const smartptr<X>& lptr, const smartptr<Y>& rptr) {
	return lptr.IsValid() && rptr.IsValid();
}

template<typename X>
bool operator&&(const smartptr<X>& lptr, bool rval) {
	return lptr.IsValid() && rval;
}

template<typename X>
bool operator&&(bool lval, const smartptr<X>& rptr) {
	return lval && rptr.IsValid();
}

template<typename X, typename Y>
bool operator||(const smartptr<X>& lptr, const smartptr<Y>& rptr) {
	return lptr.IsValid() || rptr.IsValid();
}

template<typename X>
bool operator||(const smartptr<X>& lptr, bool rval) {
	return lptr.IsValid() || rval;
}

template<typename X>
bool operator||(bool lval, const smartptr<X>& rptr) {
	return lval || rptr.IsValid();
}

template<typename X>
bool operator!(const smartptr<X>& p) {
	return (!p.IsValid());
}

/* less than comparisons for storage in containers */
template<typename X, typename Y>
bool operator<(const smartptr<X>& lptr, const smartptr<Y>& rptr) {
	return lptr.get() < rptr.get();
}

template<typename X, typename Y>
bool operator<(const smartptr<X>& lptr, Y* raw) {
	return lptr.get() < raw;
}

template<typename X, typename Y>
bool operator<(X* raw, const smartptr<Y>& rptr) {
	return raw < rptr.get();
}

#endif

