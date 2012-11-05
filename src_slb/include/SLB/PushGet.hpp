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



#ifndef __SLB_PUSH_GET__
#define __SLB_PUSH_GET__

#include "lua.hpp"
#include "Debug.hpp"

namespace SLB {

//----------------------------------------------------------------------------
//-- Push/Get/Check functions ------------------------------------------------
//----------------------------------------------------------------------------
  template<class T> void push(lua_State *L, T v);
  template<class T> T get(lua_State *L, int pos);
  template<class T> void setGlobal(lua_State *L, T v, const char *name);
  template<class T> T getGlobal(lua_State *L, const char *name);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

namespace Private {
  template<class C>
  struct Type;
}

template<class T>
inline void push(lua_State *L, T v)
{
  SLB_DEBUG_CALL; 
  Private::Type<T>::push(L,v);
}

// get function based on Private::Type<class>
template<class T>
inline T get(lua_State *L, int pos)
{
  SLB_DEBUG_CALL; 
  return Private::Type<T>::get(L,pos);
}

template<class T>
inline bool check(lua_State *L, int pos)
{
  SLB_DEBUG_CALL; 
  return Private::Type<T>::check(L,pos);
}

template<class T> 
inline void setGlobal(lua_State *L, T v, const char *name)
{
  SLB_DEBUG_CALL; 
  SLB::push(L,v);
  lua_setglobal(L,name);
}

template<class T>
inline T getGlobal(lua_State *L, const char *name)
{
  SLB_DEBUG_CALL; 
  lua_getglobal(L,name);
  T value = SLB::get<T>(L, -1);
  lua_pop(L,1); // remove the value
  return value;
}

} // end of SLB namespace

#endif
