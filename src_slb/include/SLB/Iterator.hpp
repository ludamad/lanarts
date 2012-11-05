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



#ifndef __SLB_ITERATOR__
#define __SLB_ITERATOR__

#include "Export.hpp"
#include "Object.hpp"
#include "PushGet.hpp"

struct lua_State;

namespace SLB
{

  class SLB_EXPORT IteratorBase
  {
  public:
    virtual int push(lua_State *L) = 0;
    virtual ~IteratorBase() {}
  };

  class SLB_EXPORT Iterator : public Object
  {
  public: 
    Iterator(IteratorBase *b);

  protected:
    void pushImplementation(lua_State *L);
    virtual ~Iterator();
  private:
    static int iterator_call(lua_State *L);
    IteratorBase *_iterator;
    Iterator( const Iterator &slbo);
    Iterator& operator=( const Iterator &slbo);
    SLB_CLASS(Iterator, Object)
  };


  // Standard iterator
  template<class T, class T_iterator>
  struct StdIteratorTraits
  {
    typedef T Container;
    typedef T_iterator Iterator;
    typedef Iterator (Container::*GetIteratorMember)();
    //How unref iterators:
    static typename Iterator::value_type unref(Iterator& i) { return *i; }
  };

  template<class T, class T_iterator>
  struct StdConstIteratorTraits
  {
    typedef T Container;
    typedef T_iterator Iterator;
    typedef Iterator (Container::*GetIteratorMember)() const;

    //How unref iterators:
    static const typename Iterator::value_type unref(const Iterator& i) { return *i; }
  };

  template<typename Traits>
  class StdIterator : public IteratorBase
  {
  public:
    typedef typename Traits::GetIteratorMember MemberFuncs ;
    typedef typename Traits::Container Container;
    typedef typename Traits::Iterator  Iterator;

    StdIterator(MemberFuncs m_first, MemberFuncs m_end );
    int push(lua_State *L);
  protected:
    static int next(lua_State *L) ;
  private:
    MemberFuncs _begin, _end;

    StdIterator( const StdIterator &slbo);
    StdIterator& operator=( const StdIterator &slbo);
  };

  // ------------------------------------------------------------
  // ------------------------------------------------------------
  // ------------------------------------------------------------
  
  template<class T>
  inline StdIterator<T>::StdIterator(MemberFuncs m_first, MemberFuncs m_end)
    : _begin(m_first), _end(m_end)
  {
  }
  
  template<class T>
  inline int StdIterator<T>::push(lua_State *L)
  {
    SLB_DEBUG_CALL
    Container* container = SLB::get<Container*>(L,1);
    lua_pushcclosure(L, StdIterator<T>::next, 0);
    Iterator *d = reinterpret_cast<Iterator*>(lua_newuserdata(L, sizeof(Iterator)*2));
    Iterator empty;
    memcpy(d+0,&empty,sizeof(Iterator)); // copy the image of an empty iterator into the lua memory
    memcpy(d+1,&empty,sizeof(Iterator)); // ""
    //TODO: make sure iterators doesn't need to handle destruction...
    d[0] = (container->*_begin)();
    d[1] = (container->*_end)();
    return 2;
  }

  template<class T>
  inline int StdIterator<T>::next(lua_State *L)
  {
    SLB_DEBUG_CALL
    Iterator *d = reinterpret_cast<Iterator*>(lua_touserdata(L,1));
    
    if ( d[0] != d[1] )
    {
      SLB::push(L, T::unref(d[0]) );
      d[0]++; // inc iterator
    }
    else
    {
      lua_pushnil(L);
    }
    return 1;
  }

} //end of SLB namespace

#endif
