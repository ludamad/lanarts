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


#ifndef __SLB_CLASS__
#define __SLB_CLASS__

#include "SPP.hpp"
#include "Allocator.hpp"
#include "Export.hpp"
#include "Debug.hpp"
#include "ClassInfo.hpp"
#include "ClassHelpers.hpp"
#include "Manager.hpp"
#include "FuncCall.hpp"
#include "Value.hpp"
#include "Instance.hpp"
#include "Iterator.hpp"
#include "Hybrid.hpp"
#include "String.hpp"
#include <typeinfo>
#include <map>
#include <vector>

#include <iostream>

struct lua_State;

namespace SLB {
  
  struct ClassBase
  {
    ClassBase() {}
    virtual ~ClassBase() {}
  };

  template< typename T, typename W = Instance::Default >
  class Class : public ClassBase {
  public:
    typedef Class<T,W> __Self;

    Class(const char *name, Manager *m = Manager::defaultManager());
    Class(const Class&);
    Class& operator=(const Class&);

    __Self &rawSet(const char *name, Object *obj);

    template<typename TValue>
    __Self &set(const char *name, const TValue &obj)
    { return rawSet(name, (Object*) Value::copy(obj)); }

    template<typename TValue>
    __Self &set_ref(const char *name, TValue& obj)
    { return rawSet(name, Value::ref(obj)); }

    template<typename TValue>
    __Self &set_autoDelete(const char *name, TValue *obj)
    { return rawSet(name, Value::autoDelete(obj)); }

    __Self &set(const char *name, lua_CFunction func)
    { return rawSet(name, FuncCall::create(func)); }

    template<typename TEnum>
    __Self &enumValue(const char *name, TEnum obj);

    __Self &constructor();


    /** Declares a class as hybrid, this will imply that the __index
     * and __newindex methods will be overriden, see 
     * Hybrid::registerAsHybrid */
    __Self &hybrid()
    {
      dynamic_inherits<HybridBase>();
      HybridBase::registerAsHybrid( _class );
      return *this;
    }

    template<typename TBase>
    __Self &dynamic_inherits()
    { _class->dynamicInheritsFrom<T,TBase>(); return *this;}

    template<typename TBase>
    __Self &static_inherits()
    { _class->staticInheritsFrom<T,TBase>(); return *this;}

    template<typename TBase>
    __Self &inherits()
    { return static_inherits<TBase>(); }

    template<typename T_to>
    __Self &convertibleTo( T_to* (*func)(T*)  = &(ClassConversor<T,T_to>::defaultConvert) )
    {
      _class->convertibleTo<T,T_to>(func);
      return *this;
    }

    template<typename M>
    __Self &property(const char *name, M T::*member)
    {
      _class->addProperty(name, BaseProperty::create(member));
      return *this;
    }

    /* Class__index for (non-const)methods */
    template<class C, class R, class P>
    __Self &class__index( R (C::*func)(P) )
    {
      _class->setClass__index( FuncCall::create(func) ); return *this;
    }

    /* Class__index for const methods */
    template<class C, class R, class P>
    __Self &class__index( R (C::*func)(P) const )
    {
      _class->setClass__index( FuncCall::create(func) ); return *this;
    }

    /* Class__index for C functions */
    template<class R, class P>
    __Self &class__index( R (*func)(P) )
    {
      _class->setClass__index( FuncCall::create(func) ); return *this;
    }
    
    /* Class__index for lua_functions */
    __Self &class__index(lua_CFunction func)
    {
      _class->setClass__index( FuncCall::create(func) ); return *this;
    }

    /* Class__newindex for (non-const)methods */
    template<class C, class R, class K, class V>
    __Self &class__newindex( R (C::*func)(K,V) )
    {
      _class->setClass__newindex( FuncCall::create(func) ); return *this;
    }

    /* Class__newindex for const methods */
    template<class C, class R, class K, class V>
    __Self &class__newindex( R (C::*func)(K,V) const )
    {
      _class->setClass__newindex( FuncCall::create(func) ); return *this;
    }

    /* Class__newindex for C functions */
    template<class R, class K, class V>
    __Self &class__newindex( R (*func)(K,V) )
    {
      _class->setClass__newindex( FuncCall::create(func) ); return *this;
    }
    
    /* Class__newindex for lua_functions */
    __Self &class__newindex(lua_CFunction func)
    {
      _class->setClass__newindex( FuncCall::create(func) ); return *this;
    }

    /* Object__index for (non-const)methods */
    template<class C, class R, class P>
    __Self &object__index( R (C::*func)(P) )
    {
      _class->setObject__index( FuncCall::create(func) ); return *this;
    }

    /* Object__index for const methods */
    template<class C, class R, class P>
    __Self &object__index( R (C::*func)(P) const )
    {
      _class->setObject__index( FuncCall::create(func) ); return *this;
    }

    /* Object__index for C functions */
    template<class R, class P>
    __Self &object__index( R (*func)(P) )
    {
      _class->setObject__index( FuncCall::create(func) ); return *this;
    }
    
    /* Object__index for lua_functions */
    __Self &object__index(lua_CFunction func)
    {
      _class->setObject__index( FuncCall::create(func) ); return *this;
    }

    /* Object__newindex for (non-const)methods */
    template<class C, class R, class K, class V>
    __Self &object__newindex( R (C::*func)(K,V) )
    {
      _class->setObject__newindex( FuncCall::create(func) ); return *this;
    }

    /* Object__newindex for const methods */
    template<class C, class R, class K, class V>
    __Self &object__newindex( R (C::*func)(K,V) const )
    {
      _class->setObject__newindex( FuncCall::create(func) ); return *this;
    }

    /* Object__newindex for C functions */
    template<class R, class K, class V>
    __Self &object__newindex( R (*func)(K,V) )
    {
      _class->setObject__newindex( FuncCall::create(func) ); return *this;
    }
    
    /* Object__newindex for lua_functions */
    __Self &object__newindex(lua_CFunction func)
    {
      _class->setObject__newindex( FuncCall::create(func) ); return *this;
    }

    template<class T1, class T2>
    __Self &__eq( bool (*func)(T1,T2) )
    {
      _class->setObject__newindex( FuncCall::create(func) ); return *this;
    }

    __Self &__eq(lua_CFunction func)
    {
      _class->set__eq( FuncCall::create(func) ); return *this;
    }
    
    __Self &__add()
    { SLB_DEBUG_CALL; SLB_DEBUG(0, "NOT IMPLEMENTED!"); return *this; }

    __Self &__mult()
    { SLB_DEBUG_CALL; SLB_DEBUG(0, "NOT IMPLEMENTED!"); return *this; }

    template<class IT> /* IT == Iterator Traits */
    __Self &customIterator(  const char *name,
      typename IT::GetIteratorMember first,
      typename IT::GetIteratorMember end )
    {
      Iterator* it;
      StdIterator< IT >* sit;
      sit = new (Malloc(sizeof(StdIterator<IT>))) StdIterator<IT>(first, end);
      it = new (Malloc(sizeof(Iterator))) Iterator(sit);
      return rawSet(name, it );
    }

    template<typename C, typename T_Iterator>
    __Self &iterator(const char *name,
        T_Iterator (C::*first)(),
        T_Iterator (C::*end)() )
    {
      return customIterator< StdIteratorTraits<C, T_Iterator> >
        (name,first, end ) ;
    }

    template<typename C, typename T_Iterator>
    __Self &const_iterator(const char *name,
        T_Iterator (C::*first)() const,
        T_Iterator (C::*end)() const )
    {
      return customIterator< StdConstIteratorTraits<C, T_Iterator> >
        (name,first, end ) ;
    }

    // Metada
    __Self &comment(const String&);
    __Self &param(const String&);

    #define SLB_REPEAT(N) \
    \
      /* Methods */ \
      template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      __Self &set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) ); \
      template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      __Self &nonconst_set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) ); \
    \
      /* CONST Methods */ \
      template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      __Self &set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) const ); \
      template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      __Self &const_set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) const); \
    \
      /* C-functions  */ \
      template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      __Self &set(const char *name, R (func)(SPP_ENUM_D(N,T)) ); \
    \
      /* constructors */ \
      template<class T0 SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      __Self &constructor(); \
    \
      /* Constructors (wrappers to c-functions)  */ \
      template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
      __Self &constructor(R (func)(SPP_ENUM_D(N,T)) ); \

    SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
    #undef SLB_REPEAT

  protected:
    ClassInfo *_class;
    // For metadata
    Object *_lastObj;
    size_t _param;
    Manager* _mgr;

  };
  
  template<typename T, typename W>
  inline Class<T,W>::Class(const char *name, Manager *m)
    : _class(0), _lastObj(0), _param(0), _mgr(m)
  {
    SLB_DEBUG_CALL;
    // we expect to have a template "Implementation" inside W
    typedef typename W::template Implementation<T> Adapter;
    _class = m->getOrCreateClass( _TIW(T) );
    _class->setName( name );
    typedef InstanceFactoryAdapter< T, Adapter > t_IFA;
    _class->setInstanceFactory( new (Malloc(sizeof(t_IFA))) t_IFA );
    SLB_DEBUG(1, "Class declaration for %s[%s]", name, _TIW(T).name());
  }

  template<typename T, typename W>
  inline Class<T,W>::Class(const Class &c)
    : _class(c._class), _mgr(c._mgr)
  {
  }
  
  template<typename T, typename W>
  inline Class<T,W>& Class<T,W>::operator=(const Class &c)
  {
    _class = c._class;
    _mgr = c._mgr;
  }
  
  template<typename T, typename W>
  inline Class<T,W> &Class<T,W>::rawSet(const char *name, Object *obj)
  {
    _class->set(name, obj);
    _lastObj = obj;
    _param = 0;
    return *this;
  }
  
  template<typename T,  typename W>
  inline Class<T,W> &Class<T,W>::constructor()
  {
    _class->setConstructor( FuncCall::defaultClassConstructor<T>() );
    return *this;
  }

  template<typename T, typename W>
  template<typename TEnum>
  inline Class<T,W> &Class<T,W>::enumValue(const char *name, TEnum obj)
  {
    // "fake" Declaration of TEnum...
    ClassInfo *c = _mgr->getOrCreateClass( _TIW(TEnum) );
    if (!c->initialized())
    {
      struct Aux
      {
        static bool equal(TEnum a, TEnum b) { return a == b; }
      };
      // if it is not initialized then add a simple adapter for 
      // references.
      typedef InstanceFactoryAdapter< TEnum, SLB::Instance::Default::Implementation<TEnum> > t_IFA;
      c->setInstanceFactory( new (Malloc(sizeof(t_IFA))) t_IFA );
      c->set__eq( FuncCall::create( &Aux::equal ));
    }
    // push a reference
    return rawSet(name, Value::copy(obj));
  }

  template<typename T,  typename W>
  inline Class<T,W> &Class<T,W>::comment( const String &s )
  {
    if (_lastObj) _lastObj->setInfo(s);
    else _class->setInfo(s);
    return *this;
  }

  template<typename T,  typename W>
  inline Class<T,W> &Class<T,W>::param( const String &s )
  {
    //TODO: This should also work for constructors, and so on.
    if (_lastObj)
    {
      FuncCall *fc = slb_dynamic_cast<FuncCall>(_lastObj);
      if (fc)
      {
        size_t max_param = fc->getNumArguments();
        if (_param >= max_param)
        {
        std::cerr
          << "SLB_Warning: " << fc->getInfo() <<" to many parameters (total args=" << max_param << ")" 
          << "("  << _param << ", " << s << ")"
          << std::endl;
        }
        else
        {
          fc->setArgComment(_param, s);
        }
      }
      else
      {
        std::cerr
          << "SLB_Warning: Can not set param info to a non-funcCall object " 
          << "("  << _param << ", " << s << ")"
          << std::endl;
      }
    }
    _param++;
    return *this;
  }
  #define SLB_REPEAT(N) \
  \
    /* Methods */ \
    template<typename T, typename W>\
    template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    inline Class<T,W> &Class<T,W>::set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) ){ \
      if (_TIW(T) != _TIW(C)) static_inherits<C>();\
      return rawSet(name, FuncCall::create(func)); \
    } \
    template<typename T, typename W>\
    template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    inline Class<T,W> &Class<T,W>::nonconst_set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) ){ \
      if (_TIW(T) != _TIW(C)) static_inherits<C>();\
      return rawSet(name, FuncCall::create(func)); \
    } \
  \
    /* CONST Methods */ \
    template<typename T, typename W>\
    template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    inline Class<T,W> &Class<T,W>::set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) const ){ \
      if (_TIW(T) != _TIW(C)) static_inherits<C>();\
      return rawSet(name, FuncCall::create(func)); \
    } \
    template<typename T, typename W>\
    template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    inline Class<T,W> &Class<T,W>::const_set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) const ){ \
      if (_TIW(T) != _TIW(C)) static_inherits<C>();\
      return rawSet(name, FuncCall::create(func)); \
    } \
  \
    /* C-functions  */ \
    template<typename T, typename W> \
    template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    inline Class<T,W> &Class<T,W>::set(const char *name, R (func)(SPP_ENUM_D(N,T)) ){ \
      return rawSet(name, FuncCall::create(func)); \
    } \
  \
    /* constructor  */ \
    template<typename T, typename W> \
    template<class T0 SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    inline Class<T,W> &Class<T,W>::constructor(){ \
      FuncCall *fc = FuncCall::defaultClassConstructor<T,T0 SPP_COMMA_IF(N) SPP_ENUM_D(N,T)>();\
      _class->setConstructor( fc );\
      return *this; \
    } \
    /* Constructors (wrappers c-functions)  */ \
    template<typename T, typename W> \
    template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    inline Class<T,W> &Class<T,W>::constructor( R (func)(SPP_ENUM_D(N,T)) ){ \
      _class->setConstructor( FuncCall::create( func ) ); \
      return *this;\
    } \
  \

  SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
  #undef SLB_REPEAT
}


#endif
