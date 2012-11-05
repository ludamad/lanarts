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



#include <SLB/FuncCall.hpp>
#include <SLB/Debug.hpp>
#include <SLB/Allocator.hpp>

namespace SLB {

  FuncCall::FuncCall() : _Treturn()
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(10, "Create FuncCall (%p)",this);
  }

  FuncCall::~FuncCall()
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(10, "Delete FuncCall (%p)",this);
  }

  
  void FuncCall::pushImplementation(lua_State *L)
  {
    SLB_DEBUG_CALL;
    lua_pushlightuserdata(L, (FuncCall*) this);
    lua_pushcclosure(L,FuncCall::_call, 1);
  }
  
  int FuncCall::_call(lua_State *L)
  {
    SLB_DEBUG_CALL;
    FuncCall *fc = (FuncCall*) lua_touserdata(L,lua_upvalueindex(1));
    assert("Invalid FuncCall" && fc);
#if SLB_USE_EXCEPTIONS != 0
    try
    {
      return fc->call(L);
    }
    catch ( std::exception &e )
    {
      luaL_error(L, e.what());
      return 0;
    }
#else
    return fc->call(L);
#endif
  }
  
  void FuncCall::setArgComment(size_t p, const String& c)
  {
    SLB_DEBUG_CALL;
    if (p < _Targs.size())
    {
      _Targs[p].second = c;
    }
    else
    {
      //TODO warning or exception here.
    }
  }
  

  /* For lua functions.... */
  class LuaCFunction : public FuncCall
  {
  public:
    LuaCFunction(lua_CFunction f) : _func(f) { SLB_DEBUG_CALL; }
  protected:
    virtual ~LuaCFunction() { SLB_DEBUG_CALL; }
    void pushImplementation(lua_State *L) {SLB_DEBUG_CALL; lua_pushcfunction(L,_func); }
    virtual int call(lua_State *L)
    {
      SLB_DEBUG_CALL;
      luaL_error(L, "Code should never be reached %s:%d",__FILE__,__LINE__);
      return 0;
    }
    lua_CFunction _func;
  };

  FuncCall* FuncCall::create(lua_CFunction f)
  {
    SLB_DEBUG_CALL;
    return new (Malloc(sizeof(LuaCFunction))) LuaCFunction(f);
  }

}
