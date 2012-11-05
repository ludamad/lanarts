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



#ifndef __SLB_PRIVATE_FUNC_CALL__
#define __SLB_PRIVATE_FUNC_CALL__

#include "SPP.hpp"
#include "Allocator.hpp"
#include "PushGet.hpp"
#include "ClassInfo.hpp"
#include "Manager.hpp"
#include "lua.hpp"
#include <typeinfo>


namespace SLB {
namespace Private {

//----------------------------------------------------------------------------
//-- FuncCall Implementations ------------------------------------------------
//----------------------------------------------------------------------------
  template<class T>
  class FC_Function; //> FuncCall to call functions (C static functions)

  template<class C, class T>
  class FC_Method; //> FuncCall to call Class methods

  template<class C, class T>
  class FC_ConstMethod; //> FuncCall to call Class const methods

  template<class C>
  struct FC_DefaultClassConstructor; //> FuncCall to create constructors

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  // SLB_INFO: Collects info of the arguments
  #define SLB_INFO_PARAMS(N) _Targs.push_back(\
      std::pair<TypeInfoWrapper, String>( _TIW(T##N), "") ); 
  #define SLB_INFO(RETURN, N) \
    _Treturn = _TIW(RETURN);\
    SPP_REPEAT(N,SLB_INFO_PARAMS ) \

  // SLB_GET: Generates Code to get N parameters 
  //
  //    N       --> Number of parameters
  //    START   --> where to start getting parameters
  //                n=0   means start from the top
  //                n>0   start at top+n (i.e. with objects first parameter is the object)
  //
  //    For each paramter a param_n variable is generated with type Tn
#ifdef SLB_CHECK_FUNC_ARGUMENTS
  #define SLB_GET_PARAMS(N, START) \
  if (!SLB::Private::Type<T##N>::check(L, N + (START))) { \
    luaL_error(L, "Invalid arg %d (expected %s)",N+(START),_TIW(T##N).name());\
  }\
    typename SLB::Private::Type<T##N>::GetType \
        param_##N = SLB::Private::Type<T##N>::get(L,N + (START) );
#else
  #define SLB_GET_PARAMS(N, START) \
    typename SLB::Private::Type<T##N>::GetType \
        param_##N = SLB::Private::Type<T##N>::get(L,N + (START) );
#endif

  #define SLB_GET(N,START) \
    if (lua_gettop(L) != N + (START) ) \
    { \
      luaL_error(L, "Error number of arguments (given %d -> expected %d)", \
          lua_gettop(L)-(START), N); \
    }\
    SPP_REPEAT_BASE(N,SLB_GET_PARAMS, (START) ) \
    

  // FC_Method (BODY) 
  //    N       --> Number of parameters
  //
  // ( if is a const method )
  //    NAME    --> FC_Method   |  FC_ConstMethod
  //    CONST   --> /*nothing*/ |  const 
  // 
  // ( if returns or not a value)
  //    HEADER  --> class R    |  /*nothing*/
  //    RETURN  --> R          |  void 
  //
  //    ...     --> implementation of call function
  #define SLB_FC_METHOD_BODY(N,NAME, CONST, HEADER,RETURN, ...) \
  \
    template<class C HEADER SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    class NAME <C,RETURN (SPP_ENUM_D(N,T))> : public FuncCall { \
    public: \
      NAME( RETURN (C::*func)(SPP_ENUM_D(N,T)) CONST ) : _func(func) \
      {\
        SLB_INFO(RETURN, N) \
      }\
    protected: \
      int call(lua_State *L) \
      { \
        __VA_ARGS__ \
      } \
    private: \
      RETURN (C::*_func)(SPP_ENUM_D(N,T)) CONST; \
    }; \


  // FC_Method (implementation with return or not a value):
  // ( if is a const method )
  //    NAME    --> FC_Method   |  FC_ConstMethod
  //    CONST   --> /*nothing*/ |  const 
  #define SLB_FC_METHOD(N, NAME, CONST) \
    SLB_FC_METHOD_BODY(N, NAME, CONST, SPP_COMMA class R ,R, \
        CONST C *obj = SLB::get<CONST C*>(L,1); \
        if (obj == 0) luaL_error(L, "Invalid object for this method");\
        SLB_GET(N,1) \
        R value = (obj->*_func)(SPP_ENUM_D(N,param_)); \
        SLB::push<R>(L, value); \
        return 1; \
      ) \
    SLB_FC_METHOD_BODY(N, NAME, CONST, /*nothing*/ , void,  \
        CONST C *obj = SLB::get<CONST C*>(L,1); \
        if (obj == 0) luaL_error(L, "Invalid object for this method");\
        SLB_GET(N,1) \
        (obj->*_func)(SPP_ENUM_D(N,param_)); \
        return 0; \
      )

  // FC_Method (with const methods or not)
  #define SLB_REPEAT(N) \
    SLB_FC_METHOD(N, FC_ConstMethod,  const) /* With const functions */ \
    SLB_FC_METHOD(N, FC_Method, /* nothing */ ) /* with non const functions */

  SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
  #undef SLB_REPEAT


  // FC_Function (Body)
  //    N       --> Number of parameters
  //
  // ( if returns or not a value)
  //    HEADER  --> class R    |  /*nothing*/
  //    RETURN  --> R          |  void 
  //
  //    ...     --> implementation of call function
  #define SLB_FC_FUNCTION_BODY(N, HEADER, RETURN, ...) \
  \
    template< HEADER  SPP_ENUM_D(N, class T)> \
    class FC_Function< RETURN (SPP_ENUM_D(N,T))> : public FuncCall { \
    public: \
      FC_Function( RETURN (*func)(SPP_ENUM_D(N,T)) ) : _func(func) {\
        SLB_INFO(RETURN, N) \
      } \
    protected: \
      virtual ~FC_Function() {} \
      int call(lua_State *L) \
      { \
        __VA_ARGS__ \
      } \
    private: \
      RETURN (*_func)(SPP_ENUM_D(N,T)); \
    }; 
  
  #define SLB_FC_FUNCTION(N) \
    SLB_FC_FUNCTION_BODY( N, class R SPP_COMMA_IF(N), R, \
        SLB_GET(N,0) \
        R value = (_func)(SPP_ENUM_D(N,param_)); \
        SLB::push<R>(L, value); \
        return 1; \
    )\
    SLB_FC_FUNCTION_BODY( N, /* nothing */ , void, \
        SLB_GET(N,0) \
        (_func)(SPP_ENUM_D(N,param_)); \
        return 0; \
    )

  SPP_MAIN_REPEAT_Z(MAX,SLB_FC_FUNCTION)
  #undef SLB_FC_METHOD
  #undef SLB_FC_METHOD_BODY
  #undef SLB_FC_FUNCTION
  #undef SLB_FC_FUNCTION_BODY

  #define SLB_REPEAT(N) \
    template<class C SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    struct FC_DefaultClassConstructor<C(SPP_ENUM_D(N,T))> : public FuncCall\
    {\
    public:\
      FC_DefaultClassConstructor() {} \
    protected: \
      int call(lua_State *L) \
      { \
        ClassInfo *c = Manager::getInstance(L)->getClass(_TIW(C)); \
        if (c == 0) luaL_error(L, "Class %s is not avaliable! ", _TIW(C).name()); \
        SLB_GET(N, 0); \
        Private::Type<C*>::push(L, new (Malloc(sizeof(C))) C(SPP_ENUM_D(N,param_))); \
        return 1; \
      } \
    }; \

  SPP_MAIN_REPEAT(MAX,SLB_REPEAT)
  #undef SLB_REPEAT

  // For C() like constructors (empty constructors)
  template<class C>
  struct FC_DefaultClassConstructor<C()> : public FuncCall
  {
  public:
    FC_DefaultClassConstructor() {}
  protected:
    int call(lua_State *L)
    {
      ClassInfo *c = Manager::getInstance(L)->getClass(_TIW(C));
      if (c == 0) luaL_error(L, "Class %s is not avaliable! ", _TIW(C).name());
      Private::Type<C*>::push(L, new (Malloc(sizeof(C))) C);
      return 1;
    }
  };


  #undef SLB_GET
  #undef SLB_GET_PARAMS


}} // end of SLB::Private namespace  

#endif
