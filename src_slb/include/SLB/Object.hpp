/*
    SLB - Simple Lua Binder
    Copyright (C) 2007-2011 Jose L. Hidalgo Valiño (PpluX)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  
  Jose L. Hidalgo (www.pplux.com)
  pplux@pplux.com
*/



#ifndef __SLB_OBJECT__
#define __SLB_OBJECT__

#include <assert.h>
#include "Export.hpp"
#include "Allocator.hpp"
#include "String.hpp"
#include "TypeInfoWrapper.hpp"

struct lua_State;

#define SLB_CLASS(T, Parent_T) \
  public:\
  SLB::TypeInfoWrapper typeInfo() const { return _TIW(T); } \
  const void * convertTo(const TypeInfoWrapper &tiw) const {\
    if (tiw == _TIW(T)) return this; \
    else return Parent_T::convertTo(tiw); \
  }\
  const void *memoryRawPointer() const { return this; } \

namespace SLB
{

  class SLB_EXPORT Object 
  {
  public:
    unsigned int referenceCount() const { return _refCounter; }
    void ref();
    void unref();

    void push(lua_State *L);
    void setInfo(const String&);
    const String& getInfo() const;

    virtual TypeInfoWrapper typeInfo() const = 0;
    virtual const void* convertTo(const TypeInfoWrapper &) const { return 0L; }
    virtual const void *memoryRawPointer() const = 0;

  protected:
    Object();
    virtual ~Object();

    virtual void pushImplementation(lua_State *) = 0;
    virtual void onGarbageCollection(lua_State *) {}

  private:
    void initialize(lua_State *) const;
    static int GC_callback(lua_State *);
    unsigned int _refCounter;
    String _info; // for metadata, documentation, ...

    
    Object( const Object &slbo);
    Object& operator=( const Object &slbo);
  };

  template<class T, class X>
  inline T* slb_dynamic_cast(X *obj) {
    T* result = 0L;
    if (obj)  { result = static_cast<T*>(const_cast<void*>(obj->convertTo(_TIW(T)))); }
#if SLB_USE_EXCEPTIONS != 0 // we asume exceptions means you can also use dynamic_cast
    // check the result is the same that dynamic_cast
    assert(result == dynamic_cast<T*>(obj) && "Invalid cast");
#endif
    return result;
  }

  template<class T, class X>
  inline const T* slb_dynamic_cast(const X *obj) {
    const T* result = 0L;
    if (obj)  { result = static_cast<const T*>(obj->convertTo(_TIW(T))); }
#if SLB_USE_EXCEPTIONS != 0 // we asume exceptions means you can also use dynamic_cast
    // check the result is the same that dynamic_cast
    assert(result == dynamic_cast<T*>(obj) && "Invalid cast");
#endif
    return result;
  }

  
  // ------------------------------------------------------------
  // ------------------------------------------------------------
  // ------------------------------------------------------------
    
  inline void Object::ref()
  {
    ++_refCounter;
  }

  inline void Object::unref()
  {
    assert(_refCounter > 0);
    --_refCounter; 
    if (_refCounter == 0) 
    {
      const void *ptr = memoryRawPointer();
      this->~Object();
      Free( const_cast<void*>(ptr) );
    }
  }

  inline void Object::setInfo(const String& s) {_info = s;}
  inline const String& Object::getInfo() const {return _info;}

} //end of SLB namespace

#endif
