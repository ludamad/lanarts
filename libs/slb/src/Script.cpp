/*
    SLB - Simple Lua Binder
    Copyright (C) 2007-2010 Jose L. Hidalgo Valiño (PpluX)

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


#include<SLB/Config.hpp>
#include<SLB/Script.hpp>
#include<SLB/Debug.hpp>
#include<SLB/Error.hpp>
#include<SLB/Allocator.hpp>
#include<sstream>

namespace SLB {

#if SLB_DEBUG_LEVEL != 0
  /* Debugging function, see TODO on Script::getState */
  void ScriptHook(lua_State *L, lua_Debug *ar)
  {
    lua_getinfo(L, "Sl",ar);
    __SLB_ADJUST__();
    SLB_DEBUG_FUNC("SLB-X[%p] %s:%d",L,ar->short_src,ar->currentline );
    SLB_DEBUG_FUNC("\n");\
  }
#endif

  static void DefaultPrintCallback(Script *s, const char *str, size_t length) {
    std::cout.write(str,length);
  }

  Script::Script(Manager *m) : 
    _manager(m), 
    _lua_state(0),
    _printCallback(DefaultPrintCallback),
    _allocator(&Script::allocator),
    _allocator_ud(0),
    _errorHandler(0),
    _loadDefaultLibs(true)
  {
    SLB_DEBUG_CALL;
    
    DefaultErrorHandler *err = 0;
    New_T(&err);
    _errorHandler = err;
  }

  Script::~Script()
  {
    SLB_DEBUG_CALL;
    Free_T(&_errorHandler);
    close();
  }

  void Script::setAllocator(lua_Alloc f, void *ud)
  {
    _allocator = f;
    _allocator_ud = ud;
  }
  
  lua_State* Script::getState()
  {
    SLB_DEBUG_CALL;
    if (!_lua_state)
    {
      SLB_DEBUG(10, "Open default libs = %s", _loadDefaultLibs ? " true": " false");
      _lua_state = lua_newstate(_allocator, _allocator_ud);
      assert("Can not create more lua_states" && (_lua_state != 0L));
      if (_loadDefaultLibs) luaL_openlibs(_lua_state);
      _manager->registerSLB(_lua_state);
    
      // replace print method
      lua_pushlightuserdata(_lua_state, this);
      lua_pushcclosure(_lua_state,PrintHook,1);
      lua_setglobal(_lua_state, "print");

      //TODO: Promote that functionality to a higher interface to allow proper
      //      debugging
      //
      /* if debug_level > 0 ........ */
      #if SLB_DEBUG_LEVEL != 0
      lua_sethook(_lua_state, ScriptHook, LUA_MASKLINE, 0);
      #endif
      /* end debug */

      onNewState(_lua_state);
    }
    return _lua_state;
  }

  void Script::close()
  {
    SLB_DEBUG_CALL;
    if (_lua_state)
    {
      onCloseState(_lua_state);
      lua_close(_lua_state);
      _lua_state = 0;
    }
  }

  void Script::callGC()
  {
    SLB_DEBUG_CALL;
    if (_lua_state)
    {
      onGC(_lua_state);
      lua_gc(_lua_state, LUA_GCCOLLECT, 0);
    }
  }
  
  size_t Script::memUsage()
  {
    SLB_DEBUG_CALL;
    size_t result  = 0;
    if (_lua_state)
    {
      int r = lua_gc(_lua_state, LUA_GCCOUNT, 0);
      result = r;
    }
    return result;
  }

  void Script::doFile(const char *filename) SLB_THROW((std::exception))
  { 
    if (!safeDoFile(filename)) {
      SLB_THROW(std::runtime_error( getLastError() ));
      SLB_CRITICAL_ERROR( getLastError() )
    };
  }

  bool Script::safeDoFile(const char *filename)
  {
    SLB_DEBUG_CALL;
    lua_State *L = getState();
    int top = lua_gettop(L);
    SLB_DEBUG(10, "filename %s = ", filename);
    bool result = true;

    switch(luaL_loadfile(L,filename))
    {
      case LUA_ERRFILE:
      case LUA_ERRSYNTAX:
        _lastError = lua_tostring(L, -1);
        result = false;
        break;
      case LUA_ERRMEM: 
        _lastError = "Error allocating memory";
        result = false;
        break;
    }

    // otherwise...
    if( _errorHandler->call(_lua_state, 0, 0))
    {
      _lastError = lua_tostring(L,-1);
      result = false;
    }

    lua_settop(L,top);
    return result;
  }

  void Script::doString(const char *o_code, const char *hint)  SLB_THROW((std::exception))
  {
    if (!safeDoString(o_code, hint)) {
      SLB_THROW(std::runtime_error( getLastError() ));
      SLB_CRITICAL_ERROR( getLastError() )
    };
  }

  bool Script::safeDoString(const char *o_code, const char *hint)
  {
    SLB_DEBUG_CALL;
    lua_State *L = getState();
    int top = lua_gettop(L);
    SLB_DEBUG(10, "code = %10s, hint = %s", o_code, hint); 
    std::stringstream code;
    code << "--" << hint << std::endl << o_code;
    bool result = true;

    if(luaL_loadstring(L,code.str().c_str()) || _errorHandler->call(_lua_state, 0, 0))
    {
      const char *s = lua_tostring(L,-1);
      _lastError = lua_tostring(L,-1);
    }

    lua_settop(L,top);
    return result;
  }

  void Script::setErrorHandler( ErrorHandler *e )
  {
    Free_T(&_errorHandler);
    _errorHandler = e;
  }

  void *Script::allocator(void * /*ud*/, void *ptr, size_t osize, size_t nsize)
  {
    if (nsize == 0)
    {
      SLB::Free(ptr);
      return 0;
    }
    else
    {
      void *newpos = SLB::Malloc(nsize);
      size_t count = osize;
      if (nsize < osize) count = nsize;
      if (ptr) memcpy(newpos, ptr, count);
      SLB::Free(ptr);
      return newpos;
    }
  }

  int Script::PrintHook(lua_State *L) {
    Script *script = reinterpret_cast<Script*>(lua_touserdata(L,lua_upvalueindex(1)));
    int n = lua_gettop(L);  /* number of arguments */
    int i;
    lua_getglobal(L, "tostring");
    for (i=1; i<=n; i++) {
      const char *s;
      size_t l;
      lua_pushvalue(L, -1);  /* function to be called */
      lua_pushvalue(L, i);   /* value to print */
      lua_call(L, 1, 1);
      s = lua_tolstring(L, -1, &l);  /* get result */
      if (s == NULL)
        return luaL_error(L,
           LUA_QL("tostring") " must return a string to " LUA_QL("print"));
      if (i>1) script->_printCallback(script,"\t", 1);
      script->_printCallback(script,s, l);
      lua_pop(L, 1);  /* pop result */
    }
    script->_printCallback(script,"\n",1);
    return 0;
  }

} /* SLB */
