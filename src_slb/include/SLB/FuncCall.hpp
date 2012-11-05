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



#ifndef __SLB_FUNCCALL__
#define __SLB_FUNCCALL__

#include "Object.hpp"
#include "Export.hpp"
#include "Allocator.hpp"
#include "String.hpp"
#include "SPP.hpp"
#include "lua.hpp"
#include "TypeInfoWrapper.hpp"

#include <vector>
#include <typeinfo>

namespace SLB
{
  class SLB_EXPORT FuncCall : public Object
  {
  public:

    #define SLB_REPEAT(N) \
    \
      /* FunCall for class Methods */ \
      template<class C,class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      static FuncCall* create(R (C::*func)(SPP_ENUM_D(N,T)) ); \
    \
      /* FunCall for CONST class Methods */ \
      template<class C,class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      static FuncCall* create(R (C::*func)(SPP_ENUM_D(N,T)) const ); \
    \
      /* (explicit) FunCall for CONST class Methods */ \
      template<class C,class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      static FuncCall* createConst(R (C::*func)(SPP_ENUM_D(N,T)) const ); \
    \
      /* (explicit) FunCall for NON-CONST class Methods */ \
      template<class C,class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      static FuncCall* createNonConst(R (C::*func)(SPP_ENUM_D(N,T))); \
    \
      /* FunCall for C-functions  */ \
      template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      static FuncCall* create(R (func)(SPP_ENUM_D(N,T)) ); \
    \
      /* FunCall Class constructors  */ \
      template<class C SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      static FuncCall* defaultClassConstructor(); \

    SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
    #undef SLB_REPEAT

    /* special case of a proper lua Function */
    static FuncCall* create(lua_CFunction f);

    size_t getNumArguments() const { return _Targs.size(); }
    const TypeInfoWrapper& getArgType(size_t p) const { return _Targs[p].first; }
    const String& getArgComment(size_t p) const { return _Targs[p].second; }
    const TypeInfoWrapper& getReturnedType() const { return _Treturn; }
    void setArgComment(size_t p, const String& c);

  protected:
    FuncCall();
    virtual ~FuncCall();
  
    void pushImplementation(lua_State *L);
    virtual int call(lua_State *L) = 0;

    typedef std::pair<TypeInfoWrapper, SLB::String> TypeInfoStringPair;
    std::vector< TypeInfoStringPair, Allocator<TypeInfoStringPair> > _Targs;
    TypeInfoWrapper _Treturn;
  private:
    static int _call(lua_State *L);

  friend class Manager;  
  friend class ClassInfo;
  SLB_CLASS(FuncCall, Object);
  };

} //end of SLB namespace
  
#include "FuncCall_inline.hpp"
#endif
