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

#ifndef LCOMMON_SMARTPTR_H_
#define LCOMMON_SMARTPTR_H_

#include <cstdlib>
#include "lcommon_assert.h"

typedef void (*delete_function_ptr)(void* ptr);

namespace _lcommon_private {

class smartptr_impl {
public:
	/*
	 Conversion/casting operators
	 */

	operator bool() const {
		return !empty();
	}

	/*
	 Is there only one reference on the counter?
	 */
	bool is_unique() const {
		if (refhandler && refhandler->count == 1)
			return true;
		return false;
	}

	bool empty() const {
		return !rawPtr;
	}

	void clear() {
		release();
		rawPtr = NULL;
		refhandler = NULL;
	}

	unsigned ref_count() const {
		if (refhandler)
			return refhandler->count;
		return 0;
	}

	/*
	 Destructor
	 */
	~smartptr_impl() {
		release();
	}

protected:

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

	void* rawPtr;
	RefHandler* refhandler;

	smartptr_impl(void* raw, RefHandler* c) :
			rawPtr(raw), refhandler(c) {
		if (c) {
			LCOMMON_ASSERT(raw != NULL);
			c->count++;
		}

	}


	void init(void* raw, delete_function_ptr deleter) {
		rawPtr = raw;
		if (raw) {
			refhandler = new RefHandler(deleter);
		} else {
			refhandler = NULL;
		}
	}

	smartptr_impl(void* raw, delete_function_ptr deleter) {
		init(raw, deleter);
	}

	// increment the count
	void acquire(RefHandler* c) {
		refhandler = c;
		if (c) {
			(c->count)++;
		}
	}

	// decrement the count, delete if it is 0
	void release() {
		LCOMMON_ASSERT(!refhandler || refhandler->count > 0);
		if (refhandler && --refhandler->count == 0) {
			refhandler->deleter(rawPtr);
			delete refhandler;
		}
	}
};

}

/* A smart pointer adapted from yasper that allows for definition on incomplete types */
template<typename X>
class smartptr : public _lcommon_private::smartptr_impl{
public:
	typedef X element_type;

	smartptr() :
		smartptr_impl(NULL, (RefHandler*)NULL) {
	}

	/*
	 Construct from a raw pointer
	 */
	smartptr(X* raw, RefHandler* c) :
		smartptr_impl((void*)raw, c) {
	}

	template<typename Y>
	explicit smartptr(Y* raw) :
		smartptr_impl(raw, delete_function<X>) {
	}

	template<typename Y>
	explicit smartptr(Y* raw, delete_function_ptr deleter) :
		smartptr_impl(raw, deleter) {
	}

	/*
	 Copy constructor
	 */
	smartptr(const smartptr<X>& otherPtr) :
		smartptr_impl(otherPtr.rawPtr, otherPtr.refhandler) {
	}

	template<typename Y>
	explicit smartptr(const smartptr<Y>& otherPtr) :
		smartptr_impl(otherPtr.rawPtr, otherPtr.refhandler) {
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
			rawPtr = otherPtr.get();
		}
		return *this;
	}

	/*
	 Member Access
	 */
	X* operator->() const {
		LCOMMON_ASSERT(rawPtr != NULL);
		return (X*)rawPtr;
	}

	/*
	 Dereference the pointer
	 */
	X& operator*() const {
		LCOMMON_ASSERT(rawPtr != NULL);
		return *(X*)rawPtr;
	}

	/*
	 Conversion/casting operators
	 */

	template<typename Y>
	operator smartptr<Y>() {
		//new ptr must also take our counter or else the reference counts
		//will go out of sync
		return smartptr<Y>(rawPtr, refhandler);
	}

	/* Access the raw pointer */
	X* get() const {
		LCOMMON_ASSERT(rawPtr != NULL);
		return (X*)rawPtr;
	}

	/* Obtain a new raw pointer*/
	template<typename Y>
	void set(Y* ptr) {
		release();
		init(ptr, delete_function<Y>);
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

template<typename X, typename Y>
bool operator!=(const smartptr<X>& lptr, const smartptr<Y>& rptr) {
	return (!operator==(lptr, rptr));
}

template<typename X, typename Y>
bool operator!=(const smartptr<X>& lptr, Y* raw) {
	return (!operator==(lptr, raw));
}

#endif /* LCOMMON_SMARTPTR_H_ */
