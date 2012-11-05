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


#include <SLB/Iterator.hpp>
#include <SLB/Debug.hpp>
namespace SLB {

  Iterator::Iterator(IteratorBase *b) : _iterator(b)
  {
    SLB_DEBUG_CALL;
  }

  void Iterator::pushImplementation(lua_State *L)
  {
    SLB_DEBUG_CALL;
    lua_pushlightuserdata(L, (void*) _iterator );
    lua_pushcclosure( L, Iterator::iterator_call, 1 );
  }

  Iterator::~Iterator()
  {
    SLB_DEBUG_CALL;
    Free_T(&_iterator);
  }

  int Iterator::iterator_call(lua_State *L)
  {
    SLB_DEBUG_CALL;
    IteratorBase *ib = reinterpret_cast<IteratorBase*>( lua_touserdata(L, lua_upvalueindex(1) ) );
    return ib->push(L);
  }

}
