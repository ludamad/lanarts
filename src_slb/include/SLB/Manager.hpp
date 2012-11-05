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



#ifndef __SLB_MANAGER__
#define __SLB_MANAGER__

#include "Object.hpp"
#include "ref_ptr.hpp"
#include "Export.hpp"
#include "Debug.hpp"
#include "String.hpp"
#include "TypeInfoWrapper.hpp"
#include <map>
#include <cstdlib>

namespace SLB {

  class ClassInfo;
  class Namespace;

  // copy values and objects from one lua_State to another
  bool copy(lua_State *from, int pos, lua_State *to);

  class SLB_EXPORT Manager
  {
  public:
    typedef void* (*Conversor)(void *);
    typedef SLB_Map( TypeInfoWrapper, ref_ptr<ClassInfo> ) ClassMap;
    typedef SLB_Map( String, TypeInfoWrapper ) NameMap;
    typedef std::pair<TypeInfoWrapper, TypeInfoWrapper > TypeInfoWrapperPair;
    typedef SLB_Map( TypeInfoWrapperPair, Conversor ) ConversionsMap;

    Manager();
    ~Manager();

    /** extracts the Manager registered given a luaState.
      * Returns null if in the lua_State there was no registered Manager
      * (SLB::Manager::registerSLB).
      */
    static Manager *getInstance(lua_State *L);

    /** returns the defaultManager, just in case you don't need separated managers.
        To free the memory of the default manager call destroyDefaultManager(),
        the memory is NOT automatically freed at the end of the program.  */
    static Manager *defaultManager();

    /** removes the default manager, and resets its memory. It is safe to call defaultManager before
        destruction and a new instance will be created. */
    static void destroyDefaultManager();

    const ClassInfo *getClass(const TypeInfoWrapper&) const;
    const ClassInfo *getClass(const String&) const;
    /// Returns the classInfo of an object, or null if it is a basic lua-type
    ClassInfo *getClass(lua_State *L, int pos) const ;
    ClassInfo *getClass(const TypeInfoWrapper&);
    ClassInfo *getClass(const String&);
    ClassInfo *getOrCreateClass(const TypeInfoWrapper&);

    /** Copy from one lua_State to another:
      - for basic types it will make a basic copy
      - returns true if copy was made, otherwise returns false. 
      - doesn't touch the original element

      ** WARNING **
      copy of tables is not yet implemented...
    */
    bool copy(lua_State *from, int pos, lua_State *to);

    // set a global value ( will be registered automatically on every lua_State )
    void set(const String &, Object *obj);

    // This will add a SLB table to the current global state
    void registerSLB(lua_State *L);

    // convert from C1 -> C2
    void* convert( const TypeInfoWrapper &C1, const TypeInfoWrapper &C2, void *obj);
    const void* convert( const TypeInfoWrapper &C1, const TypeInfoWrapper &C2, const void *obj);

    Namespace* getGlobals() { return _global.get(); }

    /** Returns the classMap with all defined classes */
    const ClassMap& getClassMap() const { return _classes; }

  protected:

    void rename(ClassInfo *c, const String &new_name);
    template<class Derived, class Base>
    void addConversor();
    template<class Derived, class Base>
    void addStaticConversor();
    template<class A, class B>
    void addClassConversor( Conversor );

    /** Returns the classMap with all defined classes */
    ClassMap& getClasses() { return _classes; }

    void* recursiveConvert(const TypeInfoWrapper &C1, const TypeInfoWrapper &C2, const TypeInfoWrapper& prev, void *obj);

    friend int SLB_allTypes(lua_State *);

  private:
    Manager(const Manager&);
    Manager& operator=(const Manager&);

    ClassMap _classes;
    NameMap  _names;
    ref_ptr<Namespace> _global;
    ConversionsMap _conversions;

    static Manager *_default;
    friend class ClassInfo;
  };
  
  //--------------------------------------------------------------------
  // Inline implementations:
  //--------------------------------------------------------------------
  
  template<class D, class B>
  struct ClassConversor
  {
    static void* convertToBase(void *raw_d)
    {
      D* derived = reinterpret_cast<D*>(raw_d);
      B* base = derived;
      return (void*) base;
    }
    
    static void* convertToDerived(void *raw_b)
    {
      B* base = reinterpret_cast<B*>(raw_b);
      D* derived = dynamic_cast<D*>(base);
      return (void*) derived;
    }

    static void* staticConvertToDerived(void *raw_b)
    {
      B* base = reinterpret_cast<B*>(raw_b);
      D* derived = static_cast<D*>(base);
      return (void*) derived;
    }

    static B* defaultConvert( D* ptr )
    {
      return &static_cast<B>(*ptr);
    }
    
  };

  template<class D, class B>
  inline void Manager::addConversor()
  {
    _conversions[ ConversionsMap::key_type(_TIW(D), _TIW(B)) ] = &ClassConversor<D,B>::convertToBase;
    _conversions[ ConversionsMap::key_type(_TIW(B), _TIW(D)) ] = &ClassConversor<D,B>::convertToDerived;
  }

  template<class D, class B>
  inline void Manager::addStaticConversor()
  {
    _conversions[ ConversionsMap::key_type(_TIW(D), _TIW(B)) ] = &ClassConversor<D,B>::convertToBase;
    _conversions[ ConversionsMap::key_type(_TIW(B), _TIW(D)) ] = &ClassConversor<D,B>::staticConvertToDerived;
  }

  template<class A, class B>
  inline void Manager::addClassConversor( Conversor c )
  {
    _conversions[ ConversionsMap::key_type(_TIW(A), _TIW(B)) ] = c;
  }

  inline void* Manager::convert( const TypeInfoWrapper &C1, const TypeInfoWrapper &C2, void *obj)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(10, "C1 = %s", C1.name());
    SLB_DEBUG(10, "C2 = %s", C2.name());
    SLB_DEBUG(10, "obj = %p", obj);
    if (C1 == C2)
    {
      SLB_DEBUG(11, "same class");
      return obj; 
    }

    ConversionsMap::iterator i = _conversions.find( ConversionsMap::key_type(C1,C2) );
    if (i != _conversions.end())
    {
      SLB_DEBUG(11, "directly convertible");
      return i->second( obj );
    }

    //The _conversions map only hold direct conversions added via .inherits<> or .static_inherits<>.
    // recursiveConvert can extract implied conversions from the _conversions table (but much less 
    // efficiently than a direct conversion).  For example if a direct conversion from Animal to Dog
    // exists and a conversion from Dog to Poodle exists, then recursiveConvert can convert an
    // Animal to a Poodle.
    void* result = recursiveConvert(C1, C2, TypeInfoWrapper(), obj);

    if (result)
    {
      SLB_DEBUG(11, "indirectly convertible");
    }
    else
    {
      SLB_DEBUG(11, "fail");
    }
    return result;
  }

  inline const void* Manager::convert( const TypeInfoWrapper &C1, const TypeInfoWrapper &C2, const void *obj)
  {
    return const_cast<void*>(convert(C1,C2, const_cast<void*>(obj)));
  }

  inline bool copy(lua_State *from, int pos, lua_State *to)
  {
    return Manager::getInstance(from)->copy(from,pos,to);
  }

}

#endif
