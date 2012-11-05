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



#ifndef __SLB_TYPE__
#define __SLB_TYPE__

#include "lua.hpp"
#include "Debug.hpp"
#include "SPP.hpp"
#include "Manager.hpp"
#include "ClassInfo.hpp"


namespace SLB {
namespace Private {

  // Default implementation
  template<class T>
  struct Type
  {
    typedef T GetType;

    static ClassInfo *getClass(lua_State *L)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"getClass '%s'", _TIW(T).name());
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      return c;
    }

    static void push(lua_State *L,const T &obj)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(8,"Push<T=%s>(L=%p, obj =%p)", _TIW(T).name(), L, &obj);
      getClass(L)->push_copy(L, (void*) &obj);
    }

    static T get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(8,"Get<T=%s>(L=%p, pos = %i)", _TIW(T).name(), L, pos);
      const T* obj = reinterpret_cast<const T*>( getClass(L)->get_const_ptr(L, pos) );  
      SLB_DEBUG(9,"obj = %p", obj);
      return *obj; //<-- copy ---
    }

    static bool check(lua_State *L, int pos) 
    {
      SLB_DEBUG_CALL;
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      return ((c != NULL) && (c->get_const_ptr(L, pos) != NULL));
    }

  };

  template<class T>
  struct Type<T*>
  {
    typedef T* GetType;
    static ClassInfo *getClass(lua_State *L)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"getClass '%s'", _TIW(T).name());
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      return c;
    }

    static void push(lua_State *L, T *obj)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"push '%s' of %p",
          _TIW(T).name(),
          obj);
      if (obj == 0)
      {
        lua_pushnil(L);
        return;
      }
      // use this class...  
      getClass(L)->push_ptr(L, (void*) obj);
    }

    static T* get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"get '%s' at pos %d", _TIW(T).name(), pos);
      return reinterpret_cast<T*>( getClass(L)->get_ptr(L, pos) );
    }

    static bool check(lua_State *L, int pos) 
    {
      SLB_DEBUG_CALL;
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      return (lua_isnil(L,pos) || ((c != NULL) && (c->get_ptr(L, pos) != NULL)));
    }
  };
  
  template<class T>
  struct Type<const T*>
  {
    typedef const T* GetType;
    static ClassInfo *getClass(lua_State *L)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"getClass '%s'", _TIW(T).name());
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      return c;
    }

    static void push(lua_State *L,const T *obj)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"push '%s' of %p", _TIW(T).name(), obj);
      if (obj == 0)
      {
        lua_pushnil(L);
        return;
      }
      
      getClass(L)->push_const_ptr(L, (const void*) obj);
    }

    static const T* get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"get '%s' at pos %d", _TIW(T).name(), pos);
      return reinterpret_cast<const T*>( getClass(L)->get_const_ptr(L, pos) );
    }

    static bool check(lua_State *L, int pos) 
    {
      SLB_DEBUG_CALL;
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      return (lua_isnil(L,pos) || ((c != NULL) && (c->get_const_ptr(L, pos) != NULL)));
    }

  };

  template<class T>
  struct Type<const T&>
  {
    typedef const T& GetType;
    static void push(lua_State *L,const T &obj)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"push '%s' of %p(const ref)", _TIW(T).name(), &obj);
      Type<const T*>::push(L, &obj);
    }

    static const T& get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"get '%s' at pos %d", _TIW(T).name(), pos);
      const T* obj = Type<const T*>::get(L,pos);
      return *(obj);
    }

    static bool check(lua_State *L, int pos) {
      return Type<const T*>::check(L,pos);
    }

  };
  
  template<class T>
  struct Type<T&>
  {
    typedef T& GetType;
    static ClassInfo *getClass(lua_State *L)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"getClass '%s'", _TIW(T).name());
      ClassInfo *c = SLB::Manager::getInstance(L)->getClass(_TIW(T));
      return c;
    }

    static void push(lua_State *L,T &obj)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"push '%s' of %p (reference)", _TIW(T).name(), &obj);
      getClass(L)->push_ref(L, (void*) &obj);
    }

    static T& get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(10,"get '%s' at pos %d", _TIW(T).name(), pos);
      return *(Type<T*>::get(L,pos));
    }

    static bool check(lua_State *L, int pos) {
      return Type<T*>::check(L,pos);
    }

  };

  //--- Specializations ---------------------------------------------------

  template<>
  struct Type<void*>
  {
    typedef void* GetType;
    static void push(lua_State *L,void* obj)
    {
      SLB_DEBUG_CALL; 
      if (obj == 0) lua_pushnil(L);
      else
      {
        SLB_DEBUG(8,"Push<void*> (L=%p, obj =%p)",L, obj);
        lua_pushlightuserdata(L, obj);
      }
    }

    static void *get(lua_State *L, int pos)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(8,"Get<void*> (L=%p, pos=%i ) =%p)",L, pos, lua_touserdata(L,pos));
      if (lua_islightuserdata(L,pos)) return lua_touserdata(L,pos);
      //TODO: Check here if is an userdata and convert it to void
      return 0;
    }

    static bool check(lua_State *L, int pos) {
      return (lua_isnil(L,pos) ||(lua_islightuserdata(L,pos) != 0));
    }

  };

  // Type specialization for <char>
  template<>
  struct Type<char>
  {
    typedef char GetType;
    static void push(lua_State *L, char v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push char = %d",v);
      lua_pushinteger(L,v);
    }
    static char get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      char v = (char) lua_tointeger(L,p);
      SLB_DEBUG(6,"Get char (pos %d) = %d",p,v);
      return v;
    }

    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };
  template<> struct Type<char&> : public Type<char> {};
  template<> struct Type<const char&> : public Type<char> {};

  // Type specialization for <char>
  template<>
  struct Type<unsigned char>
  {
    typedef unsigned char GetType;
    static void push(lua_State *L, unsigned char v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push unsigned char = %d",v);
      lua_pushinteger(L,v);
    }
    static unsigned char get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      unsigned char v = (unsigned char) lua_tointeger(L,p);
      SLB_DEBUG(6,"Get unsigned char (pos %d) = %d",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };
  template<> struct Type<unsigned char&> : public Type<unsigned char> {};
  template<> struct Type<const unsigned char&> : public Type<unsigned char> {};

  // Type specialization for <short>
  template<>
  struct Type<short>
  {
    typedef short GetType;
    static void push(lua_State *L, short v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push short = %d",v);
      lua_pushinteger(L,v);
    }
    static short get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      short v = (short) lua_tointeger(L,p);
      SLB_DEBUG(6,"Get short (pos %d) = %d",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };

  template<> struct Type<short&> : public Type<short> {};
  template<> struct Type<const short&> : public Type<short> {};

  // Type specialization for <unsigned short>
  template<>
  struct Type<unsigned short>
  {
    typedef unsigned short GetType;

    static void push(lua_State *L, unsigned short v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push unsigned short = %d",v);
      lua_pushinteger(L,v);
    }
    static unsigned short get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      unsigned short v = (unsigned short) lua_tointeger(L,p);
      SLB_DEBUG(6,"Get unsigned short (pos %d) = %d",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };

  template<> struct Type<unsigned short&> : public Type<unsigned short> {};
  template<> struct Type<const unsigned short&> : public Type<unsigned short> {};

  // Type specialization for <int>
  template<>
  struct Type<int>
  {
    typedef int GetType;
    static void push(lua_State *L, int v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push integer = %d",v);
      lua_pushinteger(L,v);
    }
    static int get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      int v = (int) lua_tointeger(L,p);
      SLB_DEBUG(6,"Get integer (pos %d) = %d",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };

  template<> struct Type<int&> : public Type<int> {};
  template<> struct Type<const int&> : public Type<int> {};

  // Type specialization for <unsigned int>
  template<>
  struct Type<unsigned int>
  {
    typedef unsigned int GetType;
    static void push(lua_State *L, unsigned int v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push unsigned integer = %d",v);
      lua_pushinteger(L,v);
    }
    static unsigned int get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      unsigned int v = static_cast<unsigned int>(lua_tointeger(L,p));
      SLB_DEBUG(6,"Get unsigned integer (pos %d) = %d",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };

  template<> struct Type<unsigned int&> : public Type<unsigned int> {};
  template<> struct Type<const unsigned int&> : public Type<unsigned int> {};
  

  template<>
  struct Type<long>
  {
    typedef long GetType;
    static void push(lua_State *L, long v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push long = %ld",v);
      lua_pushinteger(L,v);
    }
    static long get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      long v = (long) lua_tointeger(L,p);
      SLB_DEBUG(6,"Get long (pos %d) = %ld",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };

  template<> struct Type<long&> : public Type<long> {};
  template<> struct Type<const long&> : public Type<long> {};
  

  /* unsigned long == unsigned int */
  template<>
  struct Type<unsigned long>
  {
    typedef unsigned long GetType;
    static void push(lua_State *L, unsigned long v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push unsigned long = %lu",v);
      lua_pushnumber(L,v);
    }

    static unsigned long get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      unsigned long v = (unsigned long) lua_tonumber(L,p);
      SLB_DEBUG(6,"Get unsigned long (pos %d) = %lu",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };

  template<> struct Type<unsigned long&> : public Type<unsigned long> {};
  template<> struct Type<const unsigned long&> : public Type<unsigned long> {};
  

  template<>
  struct Type<unsigned long long>
  {
    typedef unsigned long long GetType;
    static void push(lua_State *L, unsigned long long v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push unsigned long long = %llu",v);
      lua_pushnumber(L,(lua_Number) v);
    }

    static unsigned long long get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      unsigned long long v = (unsigned long long) lua_tonumber(L,p);
      SLB_DEBUG(6,"Get unsigned long long (pos %d) = %llu",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };

  template<> struct Type<unsigned long long&> : public Type<unsigned long long> {};
  template<> struct Type<const unsigned long long&> : public Type<unsigned long long> {};
  
  // Type specialization for <double>
  template<>
  struct Type<double>
  {
    typedef double GetType;
    static void push(lua_State *L, double v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push double = %f",v);
      lua_pushnumber(L,v);
    }
    static double get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      double v = (double) lua_tonumber(L,p);
      SLB_DEBUG(6,"Get double (pos %d) = %f",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };

  template<> struct Type<double&> : public Type<double> {};
  template<> struct Type<const double&> : public Type<double> {};
  
  // Type specialization for <float>
  template<>
  struct Type<float>
  {
    typedef float GetType;
    static void push(lua_State *L, float v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push float = %f",v);
      lua_pushnumber(L,v);
    }

    static float get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      float v = (float) lua_tonumber(L,p);
      SLB_DEBUG(6,"Get float (pos %d) = %f",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };

  template<> struct Type<float&> : public Type<float> {};
  template<> struct Type<const float&> : public Type<float> {};
  
  
  // Type specialization for <bool>
  template<>
  struct Type<bool>
  {
    typedef bool GetType;
    static void push(lua_State *L, bool v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push bool = %d",(int)v);
      lua_pushboolean(L,v);
    }
    static bool get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      bool v = (lua_toboolean(L,p) != 0);
      SLB_DEBUG(6,"Get bool (pos %d) = %d",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return true; // ... well not tru, check this properly
    }
  };


  template<> struct Type<bool&> : public Type<bool> {};
  template<> struct Type<const bool&> : public Type<bool> {};

  template<>
  struct Type<std::string>
  {
    typedef std::string GetType;
    static void push(lua_State *L, const std::string &v)
    {
      SLB_DEBUG_CALL; 
      SLB_DEBUG(6, "Push const std::string& = %s",v.c_str());
      lua_pushlstring(L, v.data(), v.size());
    }

    static std::string get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      size_t len;
      const char* v = (const char*) lua_tolstring(L,p, &len);
      SLB_DEBUG(6,"Get std::string (pos %d) = %s",p,v);
      return std::string(v, len);
    }
    static bool check(lua_State *L, int pos) {
      return (lua_tostring(L,pos) != NULL);
    }
  };

  template<> struct Type<std::string&> : public Type<std::string> {};
  template<> struct Type<const std::string&> : public Type<std::string> {};


  // Type specialization for <const char*>
  template<>
  struct Type<const char*>
  {
    typedef const char* GetType;
    static void push(lua_State *L, const char* v)
    {
      SLB_DEBUG_CALL; 
      if (v)
      {
        SLB_DEBUG(6, "Push const char* = %s",v);
        lua_pushstring(L,v);
      }
      else
      {
        SLB_DEBUG(6, "Push const char* = NULL");
        lua_pushnil(L);
      }
    }

    static const char* get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      const char* v = (const char*) lua_tostring(L,p);
      SLB_DEBUG(6,"Get const char* (pos %d) = %s",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return (lua_tostring(L,pos) != NULL);
    }
  };

  template<>
  struct Type<const unsigned char*>
  {
    typedef const unsigned char* GetType;
    static void push(lua_State *L, const unsigned char* v)
    {
      SLB_DEBUG_CALL; 
      if (v)
      {
        SLB_DEBUG(6, "Push const unsigned char* = %s",v);
        lua_pushstring(L,(const char*)v);
      }
      else
      {
        SLB_DEBUG(6, "Push const unsigned char* = NULL");
        lua_pushnil(L);
      }
    }

    static const unsigned char* get(lua_State *L, int p)
    {
      SLB_DEBUG_CALL; 
      const unsigned char* v = (const unsigned char*) lua_tostring(L,p);
      SLB_DEBUG(6,"Get const unsigned char* (pos %d) = %s",p,v);
      return v;
    }
    static bool check(lua_State *L, int pos) {
      return (lua_tostring(L,pos) != NULL);
    }
  };

}} // end of SLB::Private

#endif
