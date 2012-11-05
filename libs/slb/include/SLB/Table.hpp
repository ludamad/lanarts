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



#ifndef __SLB_TABLE__
#define __SLB_TABLE__

#include "Export.hpp"
#include "Object.hpp"
#include "String.hpp"
#include "ref_ptr.hpp"
#include <typeinfo>
#include <map>
#include "lua.hpp"

namespace SLB {

  class SLB_EXPORT Table : public Object {
  public:
    typedef SLB_Map( String, ref_ptr<Object> ) Elements;
    Table(const String &separator = "", bool cacheable = false);

    void erase(const String &);
    Object* get(const String &);
    void set(const String &, Object*);

    bool isCacheable() { return _cacheable; }

    // [ -2, 0, - ] pops two elements (key, value) from the top and pushes it into
    // the cache. If _cacheable == false this won't make much sense, but you can use
    // it anyway (you can recover the values with getCache).
    void setCache(lua_State *L);

    // [ -1, +1, - ] will pop a key, and push the value or nil if not found.
    void getCache(lua_State *L);

    const Elements& getElementsMap() const { return _elements; }

  protected:
    virtual ~Table();

    Object* rawGet(const String &);
    void rawSet(const String &, Object*);
    
    void pushImplementation(lua_State *);

    /** will try to find the object, if not present will return -1. If this
     * function is not overriden not finding an object will raise an error
     * It is highly recommended to call this method in subclasses of Table
     * first.*/
    virtual int __index(lua_State *L); 
    virtual int __newindex(lua_State *L);

    virtual int __call(lua_State *L);
    virtual int __garbageCollector(lua_State *L);
    virtual int __tostring(lua_State *L);
    virtual int __eq(lua_State *L);

    Elements _elements;

    /** this function returns the index where to find the cache table that
     * __index method uses if _cacheable is true. This method must NOT be called
     * outside metamethod's implementation. */
    static int cacheTableIndex() { return lua_upvalueindex(1); }

  private:
    typedef std::pair<Table*,const String> TableFind;
    typedef int (Table::*TableMember)(lua_State*);

    int __indexProxy(lua_State *L);
    static int __meta(lua_State*);
    void pushMeta(lua_State *L, TableMember) const;

    TableFind getTable(const String &key, bool create);

    bool _cacheable;
    String _separator;

    Table(const Table&);
    Table& operator=(const Table&);
    SLB_CLASS(Table, Object);
  };

  //--------------------------------------------------------------------
  // Inline implementations:
  //--------------------------------------------------------------------
    
}


#endif
