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



#ifndef __SLB_HYBRID__
#define __SLB_HYBRID__

#include "Export.hpp"
#include "Allocator.hpp"
#include "SPP.hpp"
#include "Manager.hpp"
#include "LuaCall.hpp"
#include "Value.hpp"
#include "ClassInfo.hpp"
#include "Instance.hpp"
#include "Mutex.hpp"
#include <typeinfo>
#include <map>
#include <vector>
#include <string>

struct lua_State;

namespace SLB {

  class HybridBase;
    
  struct SLB_EXPORT InvalidMethod : public std::exception
  {  
    InvalidMethod(const HybridBase*, const char *c);
    ~InvalidMethod() throw() {}
    const char* what() const throw() { return _what.c_str(); }
    std::string _what;
  };

  class SLB_EXPORT HybridBase {
  public:

    /** Returns the lua_State, this function will be valid if the object is
     * attached, otherwise will return 0 */
    virtual lua_State* getLuaState() const { return _lua_state; }

    /** Indicates where this instance will look for its hybrid methods; If you reimplement
     this method remember to call the parent (HybridBase) to set _lua_state properly and register
     itself there.*/
    virtual void attach(lua_State *);
    virtual bool isAttached() const { return (_lua_state != 0); }

    /** use this to release memory allocated by the hybrid object, inside
     * the lua_State.*/
    void unAttach(); 

    /** Use this function to register this class as hybrid, it will override
     * ClassInfo metamethods of class__index, class__newindex, object__index and
         * object__newindex. 
     * Note: if your class requires those methods contact me to see if it is possible
     * to do it, by the moment this is the only way this works */
    static void registerAsHybrid(ClassInfo *ci);

  protected:
    typedef SLB_Map(String, LuaCallBase *) MethodMap;

    HybridBase();
    virtual ~HybridBase();

    //-- Private data -----------------------------------------------------
    // [-1, (0|+1)]
    bool getMethod(const char *name) const;
    virtual ClassInfo* getClassInfo() const = 0;
    void clearMethodMap();


    mutable MethodMap _methods;
    mutable ref_ptr<Table> _subclassMethods;

    friend struct InternalHybridSubclass;
    friend struct InvalidMethod;
    //-- Private data -----------------------------------------------------

  private:
    lua_State * _lua_state;
    int _data; //< lua ref to internal data

    // pops a key,value from tom and sets as our method
    // [-2,0]
    static void setMethod(lua_State *L, ClassInfo *ci);

    static int call_lua_method(lua_State *L);
    static int class__newindex(lua_State *);
    static int class__index(lua_State *);
    static int object__index(lua_State *);
    static int object__newindex(lua_State *);

  protected:
    mutable Mutex _mutex;
  };

  template<class BaseClass, class T_CriticalSection = ActiveWaitCriticalSection >
  class Hybrid : public virtual HybridBase {
  public:
    Hybrid(Manager* mgr = Manager::defaultManager())
      : _mgr(mgr)
    {
      ClassInfo *c;
      c = _mgr->getOrCreateClass( _TIW(BaseClass) );
      if (!c->initialized())
      {
        // Give a default instance factory... that only is able
        // to handle push/get of pointers without handling 
        // construction, copy, delete, ...
        typedef InstanceFactoryAdapter< BaseClass,
          Instance::NoCopyNoDestroy::Implementation<BaseClass> > t_IFA;
        c->setInstanceFactory( new (Malloc(sizeof(t_IFA))) t_IFA);
      }
    }
  private:
    Manager* _mgr;
  protected:
    virtual ~Hybrid() {}
    ClassInfo* getClassInfo() const
    {
      return _mgr->getClass( _TIW(BaseClass) );
    }
    
  #define SLB_ARG_H(N) ,T##N arg_##N
  #define SLB_ARG(N) , arg_##N
  #define SLB_BODY(N) \
      \
      T_CriticalSection __dummy__lock(&_mutex); \
      LC *method = 0; \
      SLB_DEBUG(3,"Call Hybrid-method [%s]", name)\
      lua_State *L = getLuaState(); \
      {\
        SLB_DEBUG_CLEAN_STACK(L,0)\
        MethodMap::iterator it = _methods.find(name) ; \
        if (it != _methods.end()) \
        { \
          method = reinterpret_cast<LC*>(it->second); \
          SLB_DEBUG(4,"method [%s] was found %p", name,method)\
        } \
        else \
        { \
          if (getMethod(name)) \
          { \
            method = new (Malloc(sizeof(LC))) LC(L, -1);\
            lua_pop(L,1); /*method is stored in the luaCall*/\
            SLB_DEBUG(2,"method [%s] found in lua [OK] -> %p", name,method)\
            _methods[name] = method;\
          } \
          else \
          { \
            _methods[name] = 0L; \
            SLB_DEBUG(2,"method [%s] found in lua [FAIL!]", name)\
          }\
        }\
        if (!method) {\
          SLB_THROW(InvalidMethod(this, name));\
          SLB_CRITICAL_ERROR("Invalid method")\
        }\
      }\

  #define SLB_REPEAT(N) \
  \
    /* non const version */\
    template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    R LCall( const char *name SPP_REPEAT(N, SLB_ARG_H) ) \
    { \
      SLB_DEBUG_CALL;\
      typedef SLB::LuaCall<R(BaseClass* SPP_COMMA_IF(N) SPP_ENUM_D(N,T))> LC;\
      SLB_BODY(N) \
      return (*method)(static_cast<BaseClass*>(this) SPP_REPEAT(N, SLB_ARG) ); \
    } \
    /* const version */\
    template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    R LCall( const char *name SPP_REPEAT(N, SLB_ARG_H) ) const \
    { \
      SLB_DEBUG_CALL;\
      typedef SLB::LuaCall<R(const BaseClass* SPP_COMMA_IF(N) SPP_ENUM_D(N,T))> LC;\
      SLB_BODY(N) \
      return (*method)(static_cast<const BaseClass*>(this) SPP_REPEAT(N, SLB_ARG) ); \
    } \
    /* (VOID) non const version */\
    SPP_IF(N,template<SPP_ENUM_D(N, class T)>) \
    void void_LCall( const char *name SPP_REPEAT(N, SLB_ARG_H) ) \
    { \
      SLB_DEBUG_CALL;\
      typedef SLB::LuaCall<void(BaseClass* SPP_COMMA_IF(N) SPP_ENUM_D(N,T))> LC;\
      SLB_BODY(N) \
      (*method)(static_cast<BaseClass*>(this) SPP_REPEAT(N, SLB_ARG) ); \
    } \
    /* (VOID) const version */\
    SPP_IF(N, template<SPP_ENUM_D(N, class T)>) \
    void void_LCall( const char *name SPP_REPEAT(N, SLB_ARG_H) ) const \
    { \
      SLB_DEBUG_CALL;\
      typedef SLB::LuaCall<void(const BaseClass* SPP_COMMA_IF(N) SPP_ENUM_D(N,T))> LC;\
      SLB_BODY(N) \
      return (*method)(static_cast<const BaseClass*>(this) SPP_REPEAT(N, SLB_ARG) ); \
    } \

  SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
  #undef SLB_REPEAT
  #undef SLB_BODY
  #undef SLB_ARG
  #undef SLB_ARG_H
  };
}

#define HYBRID_method_0(name,ret_T) \
  ret_T name() { SLB_DEBUG_CALL; return LCall<ret_T>(#name); }
#define HYBRID_method_1(name,ret_T, T1) \
  ret_T name(T1 p1) { SLB_DEBUG_CALL; return LCall<ret_T,T1>(#name,p1); }
#define HYBRID_method_2(name,ret_T, T1, T2) \
  ret_T name(T1 p1,T2 p2) { SLB_DEBUG_CALL; return LCall<ret_T,T1,T2>(#name,p1,p2); }
#define HYBRID_method_3(name,ret_T, T1, T2, T3) \
  ret_T name(T1 p1,T2 p2, T3 p3) { SLB_DEBUG_CALL; return LCall<ret_T,T1,T2,T3>(#name,p1,p2, p3); }
#define HYBRID_method_4(name,ret_T, T1, T2, T3, T4) \
  ret_T name(T1 p1,T2 p2, T3 p3, T4 p4) {  SLB_DEBUG_CALL; return LCall<ret_T,T1,T2,T3,T4>(#name,p1,p2, p3,p4); }
#define HYBRID_method_5(name,ret_T, T1, T2, T3, T4,T5) \
  ret_T name(T1 p1,T2 p2, T3 p3, T4 p4, T5 p5) {  SLB_DEBUG_CALL; return LCall<ret_T,T1,T2,T3,T4,T5>(#name,p1,p2, p3,p4,p5); }

#define HYBRID_const_method_0(name,ret_T) \
  ret_T name() const {  SLB_DEBUG_CALL; return LCall<ret_T>(#name); }
#define HYBRID_const_method_1(name,ret_T, T1) \
  ret_T name(T1 p1) const {  SLB_DEBUG_CALL; return LCall<ret_T,T1>(#name,p1); }
#define HYBRID_const_method_2(name,ret_T, T1, T2) \
  ret_T name(T1 p1,T2 p2) const {  SLB_DEBUG_CALL; return LCall<ret_T,T1,T2>(#name,p1,p2); }
#define HYBRID_const_method_3(name,ret_T, T1, T2, T3) \
  ret_T name(T1 p1,T2 p2, T3 p3) const {  SLB_DEBUG_CALL; return LCall<ret_T,T1,T2,T3>(#name,p1,p2, p3); }
#define HYBRID_const_method_4(name,ret_T, T1, T2, T3, T4) \
  ret_T name(T1 p1,T2 p2, T3 p3, T4 p4) const {  SLB_DEBUG_CALL; return LCall<ret_T,T1,T2,T3,T4>(#name,p1,p2, p3,p4); }
#define HYBRID_const_method_5(name,ret_T, T1, T2, T3, T4,T5) \
  ret_T name(T1 p1,T2 p2, T3 p3, T4 p4, T5 p5) const {  SLB_DEBUG_CALL; return LCall<ret_T,T1,T2,T3,T4,T5>(#name,p1,p2, p3,p4,p5); }

#define HYBRID_void_method_0(name) \
  void name() { SLB_DEBUG_CALL; return void_LCall(#name); }
#define HYBRID_void_method_1(name, T1) \
  void name(T1 p1) { SLB_DEBUG_CALL; return void_LCall<T1>(#name,p1); }
#define HYBRID_void_method_2(name, T1, T2) \
  void name(T1 p1,T2 p2) { SLB_DEBUG_CALL; return void_LCall<T1,T2>(#name,p1,p2); }
#define HYBRID_void_method_3(name, T1, T2, T3) \
  void name(T1 p1,T2 p2, T3 p3) { SLB_DEBUG_CALL; return void_LCall<T1,T2,T3>(#name,p1,p2, p3); }
#define HYBRID_void_method_4(name, T1, T2, T3, T4) \
  void name(T1 p1,T2 p2, T3 p3, T4 p4) {  SLB_DEBUG_CALL; return void_LCall<T1,T2,T3,T4>(#name,p1,p2, p3,p4); }
#define HYBRID_void_method_5(name, T1, T2, T3, T4,T5) \
  void name(T1 p1,T2 p2, T3 p3, T4 p4, T5 p5) {  SLB_DEBUG_CALL; return void_LCall<T1,T2,T3,T4,T5>(#name,p1,p2, p3,p4,p5); }

#define HYBRID_const_void_method_0(name) \
  void name() const {  SLB_DEBUG_CALL; return void_LCall(#name); }
#define HYBRID_const_void_method_1(name, T1) \
  void name(T1 p1) const {  SLB_DEBUG_CALL; return void_LCall<T1>(#name,p1); }
#define HYBRID_const_void_method_2(name, T1, T2) \
  void name(T1 p1,T2 p2) const {  SLB_DEBUG_CALL; return void_LCall<T1,T2>(#name,p1,p2); }
#define HYBRID_const_void_method_3(name, T1, T2, T3) \
  void name(T1 p1,T2 p2, T3 p3) const {  SLB_DEBUG_CALL; return void_LCall<T1,T2,T3>(#name,p1,p2, p3); }
#define HYBRID_const_void_method_4(name, T1, T2, T3, T4) \
  void name(T1 p1,T2 p2, T3 p3, T4 p4) const {  SLB_DEBUG_CALL; return void_LCall<T1,T2,T3,T4>(#name,p1,p2, p3,p4); }
#define HYBRID_const_void_method_5(name, T1, T2, T3, T4,T5) \
  void name(T1 p1,T2 p2, T3 p3, T4 p4, T5 p5) const {  SLB_DEBUG_CALL; return void_LCall<T1,T2,T3,T4,T5>(#name,p1,p2, p3,p4,p5); }

#endif
