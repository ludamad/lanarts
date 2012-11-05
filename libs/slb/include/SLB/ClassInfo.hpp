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



#ifndef __SLB_CLASS_INFO__
#define __SLB_CLASS_INFO__

#include "Export.hpp"
#include "Object.hpp"
#include "Instance.hpp"
#include "Table.hpp"
#include "ref_ptr.hpp"
#include "FuncCall.hpp"
#include "String.hpp"
#include "Property.hpp"
//#include "ClassHelpers.hpp"
#include <typeinfo>
#include <vector>

struct lua_State;

namespace SLB {

  class SLB_EXPORT Namespace : public Table
  {
  public:
    Namespace( bool cacheable = true ) : Table("::", cacheable) {}
  protected:
    virtual ~Namespace() {}
    SLB_CLASS(Namespace, Table);
  };

  class SLB_EXPORT ClassInfo : public Namespace
  {
  public:
    typedef SLB_Map(TypeInfoWrapper, ClassInfo* ) BaseClassMap;
    
    const TypeInfoWrapper &getTypeid() const { return __TIW; }
    const String &getName() const      { return _name; }
    void setName(const String&);

    void push_ref(lua_State *L, void *ref);
    void push_ptr(lua_State *L, void *ptr);
    void push_const_ptr(lua_State *L, const void *const_ptr);
    void push_copy(lua_State *L, const void *ptr);
    void* get_ptr(lua_State*, int pos) const;
    const void* get_const_ptr(lua_State*, int pos) const;

    // Uses dynamic_cast to convert from Base to Derived
    template<class This, class Base>
    void dynamicInheritsFrom();

    // This version uses static cast instead of dynamic_cast
    template<class This, class Base>
    void staticInheritsFrom();

    template<class This, class Other>
    void convertibleTo( Other* (*func)(This*) );


    void setConstructor( FuncCall *constructor );
    void setInstanceFactory( InstanceFactory *);

    /** __index method will receive:
     *  - object
     *  - key */
    void setObject__index( FuncCall* );

    /** __index method will receive:
     *  - object
     *  - key
     *  - value */
    void setObject__newindex( FuncCall* );

    /** Here you can use setCache/getCache methods to
     * speed up indexing.
     *
     * __index method will receive:
     *  - [table] -> ClassInfo
     *  - key */
    void setClass__index( FuncCall* );

    /** Here you can use setCache/getCache methods to
     * speed up indexing.
     * __index method will receive:
     *  - [table] -> ClassInfo
     *  - key
     *  - value */
    void setClass__newindex( FuncCall* );

    /** __eq method will receive to objects, and should return
      * true or false if those objects are equal or not. */
    void set__eq( FuncCall* );

    //This is used by some default initializations...
    bool initialized() const { return _instanceFactory != 0; }

    bool isSubClassOf( const ClassInfo* );
    bool hasConstructor() const { return _constructor.valid(); }

    //--Private methods -(not meant to be used)-------------------
    void setHybrid() { _isHybrid = true; }
    FuncCall* getConstructor() { return _constructor.get(); }

    // to add properties
    void addProperty(const String &name, BaseProperty *prop)
    {
      _properties[name] = prop;
    }

    BaseProperty* getProperty(const String &key);

    // const getters
    const BaseClassMap& getBaseClasses() const { return _baseClasses; }
    const FuncCall* getConstructor() const { return _constructor.get(); }

  protected:
    // Class Info are crated using manager->getOrCreateClass()
    ClassInfo(Manager *m, const TypeInfoWrapper &);
    virtual ~ClassInfo();
    void pushImplementation(lua_State *);
    virtual int __index(lua_State*);
    virtual int __newindex(lua_State*);
    virtual int __call(lua_State*);
    virtual int __garbageCollector(lua_State*);
    virtual int __tostring(lua_State*);
    virtual int __eq(lua_State *L);

    Manager          *_manager;
    TypeInfoWrapper   __TIW;
    String            _name;
    InstanceFactory  *_instanceFactory;
    BaseClassMap      _baseClasses;
    BaseProperty::Map _properties;
    ref_ptr<FuncCall> _constructor;
    ref_ptr<FuncCall> _meta__index[2];    // 0 = class, 1 = object
    ref_ptr<FuncCall> _meta__newindex[2]; // 0 = class, 1 = object
    ref_ptr<FuncCall> _meta__eq; 
    bool _isHybrid;

  private:
    void pushInstance(lua_State *L, InstanceBase *instance);
    InstanceBase* getInstance(lua_State *L, int pos) const;

    friend class Manager;
    SLB_CLASS(ClassInfo, Namespace);
  };


  //--------------------------------------------------------------------
  // Inline implementations:
  //--------------------------------------------------------------------
  
    
  template<class D, class B>
  inline void ClassInfo::dynamicInheritsFrom()
  {
    _manager->template addConversor<D,B>();
    ClassInfo *ci = _manager->getOrCreateClass(_TIW(B));
    assert( (ci != this) && "Circular reference between ClassInfo classes");
    _baseClasses[ _TIW(B) ] = ci;
  }

  template<class D, class B>
  inline void ClassInfo::staticInheritsFrom()
  {
    _manager->template addStaticConversor<D,B>();
    ClassInfo *ci = _manager->getOrCreateClass(_TIW(B));
    assert( (ci != this) && "Circular reference between ClassInfo classes");
    _baseClasses[ _TIW(B) ] = ci;
  }

  template<class This, class Other>
  inline void ClassInfo::convertibleTo( Other* (*func)(This*) )
  {
    /* This is a pretty ugly cast, SLB Manager handles changes from one type to other through
       void*, here we are changing the function to receive two pointers from the expected types (when the
       origin will be void*) It should be completely safe, as a pointer is the same size no matter the type. 

       Anyway, it's a pretty ugly trick. */
    typedef void* (*T_void)(void*);
    T_void aux_f = reinterpret_cast<T_void>(func);

    /* Once we've forced the conversion of the function we can let the manager know how to change from
       this type to the other one */
    _manager->template addClassConversor<This,Other>(aux_f);
    ClassInfo *ci = _manager->getOrCreateClass(_TIW(Other));
    assert( (ci != this) && "Circular reference between ClassInfo classes");
    _baseClasses[ _TIW(Other) ] = ci;
  }

}


#endif
