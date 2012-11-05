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



#ifndef __SLB_FUNCCALL_INLINE__
#define __SLB_FUNCCALL_INLINE__

#include "Private_FuncCall.hpp"

namespace SLB {

  #define SLB_REPEAT(N) \
  \
  template<class C,class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
  inline FuncCall* FuncCall::create(R (C::*func)(SPP_ENUM_D(N,T)) ) \
  { \
    return createNonConst(func); \
  } \
  \
  template<class C,class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
  inline FuncCall* FuncCall::create(R (C::*func)(SPP_ENUM_D(N,T)) const ) \
  { \
    return createConst(func); \
  } \
  \
  template<class C,class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
  inline FuncCall* FuncCall::createConst(R (C::*func)(SPP_ENUM_D(N,T)) const ) \
  { \
    typedef Private::FC_ConstMethod<C,R(SPP_ENUM_D(N,T))> _type_;\
    return new (Malloc(sizeof(_type_))) _type_(func); \
  } \
  template<class C,class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
  inline FuncCall* FuncCall::createNonConst(R (C::*func)(SPP_ENUM_D(N,T)) ) \
  { \
    typedef Private::FC_Method<C,R(SPP_ENUM_D(N,T))> _type_;\
    return new (Malloc(sizeof(_type_))) _type_(func); \
  } \
  \
  template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
  inline FuncCall* FuncCall::create(R (*func)(SPP_ENUM_D(N,T)) ) \
  { \
    typedef Private::FC_Function<R(SPP_ENUM_D(N,T))> _type_;\
    return new (Malloc(sizeof(_type_))) _type_(func); \
  } \
  \
  template<class C SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
  inline FuncCall* FuncCall::defaultClassConstructor() \
  { \
    typedef Private::FC_DefaultClassConstructor<C(SPP_ENUM_D(N,T))> _type_;\
    return new (Malloc(sizeof(_type_))) _type_; \
  } \
  
  SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
  #undef SLB_REPEAT

}
#endif
