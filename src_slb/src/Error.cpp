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



#include <SLB/Debug.hpp>
#include <SLB/Error.hpp>
#include <sstream>
#include <cassert>

namespace SLB {

////////////////////////////////////////////////////////////////////////////////
// Abstract Error Handler
////////////////////////////////////////////////////////////////////////////////

int ErrorHandler::call(lua_State *L, int nargs, int nresults)
{
  SLB_DEBUG_CALL;

  // insert the error handler before the arguments
  int base = lua_gettop(L) - nargs;
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, _slb_stackHandler, 1);
  lua_insert(L, base);

  // call the function
  int result = lua_pcall(L, nargs, nresults, base);

  // remove the error handler
  lua_remove(L, base);

  // return the previous result
  return result;
}

const char *ErrorHandler::SE_name()
{
  if (_lua_state) return _debug.name;
  return NULL;
}

const char *ErrorHandler::SE_nameWhat()
{
  if (_lua_state) return _debug.namewhat;
  return NULL;
}

const char *ErrorHandler::SE_what()
{
  if (_lua_state) return _debug.what;
  return NULL;
}

const char *ErrorHandler::SE_source()
{
  if (_lua_state) return _debug.source;
  return NULL;
}

const char *ErrorHandler::SE_shortSource()
{
  if (_lua_state) return _debug.short_src;
  return NULL;
}

int ErrorHandler::SE_currentLine()
{
  if (_lua_state) return _debug.currentline;
  return -1;
}

int ErrorHandler::SE_numberOfUpvalues()
{
  if (_lua_state) return _debug.nups;
  return -1;
}

int ErrorHandler::SE_lineDefined()
{
  if (_lua_state) return _debug.linedefined;
  return -1;
}

int ErrorHandler::SE_lastLineDefined()
{
  if (_lua_state) return _debug.lastlinedefined;
  return -1;
}

int ErrorHandler::_slb_stackHandler(lua_State *L)
{
  ErrorHandler *EH = reinterpret_cast<ErrorHandler*>(lua_touserdata(L,lua_upvalueindex(1)));
  if (EH) EH->process(L);
  return 1;
}

void ErrorHandler::process(lua_State *L)
{
  _lua_state = L;
  assert("Invalid state" && _lua_state != 0);
  const char *error = lua_tostring(_lua_state, -1);
  begin(error);
  for ( int level = 0; lua_getstack(_lua_state, level, &_debug ); level++)
  {
    if (lua_getinfo(L, "Slnu", &_debug) )
    {
      stackElement(level);  
    }
    else
    {
      // Throw an exception here ?
      assert("[ERROR using Lua DEBUG INTERFACE]" && false);
    }
  }
  const char *msg = end();
  lua_pushstring(_lua_state, msg);
  _lua_state = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Default Error Handler
////////////////////////////////////////////////////////////////////////////////

void DefaultErrorHandler::begin(const char *error)
{
  _out.clear();
  _out.str("");
  _out << "SLB Exception: "
    << std::endl << "-------------------------------------------------------"
    << std::endl;
  _out << "Lua Error:" << std::endl << "\t" 
    <<  error << std::endl
    << "Traceback:" << std::endl;
}

const char* DefaultErrorHandler::end()
{
  _final = _out.str();
  return _final.c_str();
}

void DefaultErrorHandler::stackElement(int level)
{
  _out << "\t [ " << level << " (" << SE_what() << ") ] ";
  int currentline = SE_currentLine();
  if (currentline > 0 )
  {
    _out << SE_shortSource() << ":" << currentline; 
  }
  const char *name = SE_name();
  if (name)
  {
    _out << " @ " << name;
     if (SE_nameWhat()) _out << "(" << SE_nameWhat() << ")";
  }
  _out << std::endl;
}

} /* SLB */
