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


#include "Config.hpp"

#ifndef __SLB_DEBUG__
#define __SLB_DEBUG__

  
#if SLB_DEBUG_LEVEL != 0
  #include "SPP.hpp"
  #include "Export.hpp"

  extern SLB_EXPORT int SLB_DEBUG_LEVEL_TAB;

  inline void __SLB_ADJUST__(bool terminator = true)
  {
    if (SLB_DEBUG_LEVEL_TAB)
    {
      for(int i = 0; i < SLB_DEBUG_LEVEL_TAB-1; ++i) SLB_DEBUG_FUNC("| ");
      if (terminator) SLB_DEBUG_FUNC("|_");
    }
  }

  #define SLB_DEBUG(level,...) if (level <= SLB_DEBUG_LEVEL)\
    {\
      __dummy__SLB__debugcall.check_SLB_DEBUG_CALL(); /* to check a previous SLB_DEBUG_CALL */ \
      __SLB_ADJUST__();\
      SLB_DEBUG_FUNC("SLB-%-2d ", level);\
      SLB_DEBUG_FUNC(__VA_ARGS__);\
      SLB_DEBUG_FUNC(" [%s:%d]\n",__FILE__, __LINE__);\
    }

  #define SLB_DEBUG_STACK(level, L,  ... ) \
    {\
      SLB_DEBUG(level, " {stack} "  __VA_ARGS__ );\
      int top = lua_gettop(L); \
      for(int i = 1; i <= top; i++) \
      { \
        if (lua_type(L,i) == LUA_TNONE) \
        { \
          SLB_DEBUG(level, "\targ %d = (Invalid)", i);\
        } \
        else \
        { \
          lua_pushvalue(L,i);\
          SLB_DEBUG(level+1, "\targ %d = %s -> %s", i, \
            lua_typename(L,lua_type(L,-1)), lua_tostring(L,-1) );\
          lua_pop(L,1);\
        } \
      }\
    }

    #include<sstream>
    #include<stdexcept>
    #include "lua.hpp"
  
    struct __SLB__cleanstack
    {
      __SLB__cleanstack(struct lua_State *L, int delta, const char *where, int line)
        : L(L), delta(delta), where(where), line(line)
      {
        top = lua_gettop(L);
      }
      ~__SLB__cleanstack()
      {
        if (top+delta != lua_gettop(L))
        {
          std::ostringstream out;
          out << where << ":" << line << " -> ";
          out << "Invalid Stack Check. current = " << lua_gettop(L) << " expected = " << top + delta << std::endl;
          SLB_THROW(std::runtime_error(out.str()));
          SLB_CRITICAL_ERROR(out.str().c_str());
        }
      }

      struct lua_State *L;
      int top;
      int delta;
      const char *where;
      int line;
    };

  #define SLB_DEBUG_CLEAN_STACK(Lua_Stack_L, delta)  \
    __SLB__cleanstack __dummy__SLB__cleanstack__(Lua_Stack_L, delta, __FILE__, __LINE__);

    struct __SLB__debugcall
    {
      __SLB__debugcall(const char *f, int l, const char *n)
        : file(f), line(l), name(n)
      {
        __SLB_ADJUST__();
        SLB_DEBUG_FUNC("SLB >>> %s [%s:%d]\n", name, file, line);
        SLB_DEBUG_LEVEL_TAB++;
      }

      ~__SLB__debugcall()
      {
        __SLB_ADJUST__();
        SLB_DEBUG_FUNC("SLB <<< %s [%s:%d]\n", name, file, line);
        __SLB_ADJUST__(false);
        SLB_DEBUG_FUNC("\n");
        SLB_DEBUG_LEVEL_TAB--;
      }

      void check_SLB_DEBUG_CALL() const {}


      const char* file;
      int line;
      const char* name;

    };
  #define SLB_DEBUG_CALL \
    __SLB__debugcall __dummy__SLB__debugcall(__FILE__,__LINE__,__FUNCTION__);

#else
  #define SLB_DEBUG(level,...)
  #define SLB_DEBUG_STACK(... ) 
  #define SLB_DEBUG_CLEAN_STACK(...)
  #define SLB_DEBUG_CALL
#endif

#endif
