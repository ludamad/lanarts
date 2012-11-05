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

/*
   Based on Steven Lavavej's "Mallocator"
   http://blogs.msdn.com/b/vcblog/archive/2008/08/28/the-mallocator.aspx

   This is a stateless global allocator that redirects all memory 
   allocation to user supplied "malloc" and "free" functions.
   The default implementation actually uses the global malloc and free
   functions.  The user supplied functions must be registered before
   using any SLB component.
*/
#ifndef __SLB_ALLOCATOR__
#define __SLB_ALLOCATOR__

#include <stddef.h>  // Required for size_t and ptrdiff_t
#include <new>       // Required for placement new
#include <stdexcept> // Required for std::length_error

#include "Export.hpp"

// To define Maps using our allocator
#define SLB_Map(Key,T) std::map<Key, T, std::less<Key>, Allocator< std::pair<const Key, T> > >

namespace SLB
{
  typedef void* (*MallocFn)(size_t);
  typedef void (*FreeFn)(void*);

  SLB_EXPORT void SetMemoryManagement(MallocFn, FreeFn);

  SLB_EXPORT void* Malloc(size_t);
  SLB_EXPORT void Free(void*);

  //Generic stateless allocator that uses the SLB::Malloc and 
  // SLB::Free functions for memory management
  template <typename T>
  class Allocator
  {
  public:
    typedef T * pointer;
    typedef const T * const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    /*
    T * address(T& r) const 
    {
      return &r;
    }
    */

    const T * address(const T& s) const 
    {
      return &s;
    }


    size_t max_size() const 
    {
      // The following has been carefully written to be independent of
      // the definition of size_t and to avoid signed/unsigned warnings.
      return (static_cast<size_t>(0) - static_cast<size_t>(1)) / sizeof(T);
    }

    template <typename U> struct rebind 
    {
      typedef Allocator<U> other;
    };

    bool operator!=(const Allocator& other) const
    {
      return !(*this == other);
    }

    void construct(T * const p, const T& t) const 
    {
      void * const pv = static_cast<void *>(p);
      new (pv) T(t);
    }

    #if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4100) // unreferenced formal parameter
    #endif
    void destroy(T * const p) const
    {
      p->~T();
    }
    #if defined(_MSC_VER)
    #pragma warning(pop)
    #endif

    bool operator==(const Allocator& /*other*/) const 
    {
      return true;
    }

    Allocator() { }
    Allocator(const Allocator&) { }
    template <typename U>
    Allocator(const Allocator<U>&) { }
    ~Allocator() { }

    T * allocate(const size_t n) const
    {
      if (n == 0)
      {
        return NULL;
      }

      if (n > max_size())
      {
        SLB_THROW(std::length_error("Allocator<T>::allocate() - Integer overflow."));
        SLB_CRITICAL_ERROR("Allocator<T>::allocate() - Integer overflow.");
      }

      void * const pv = Malloc(n * sizeof(T));
      if (pv == NULL) 
      {
        SLB_THROW(std::bad_alloc());
        SLB_CRITICAL_ERROR("Out of memory");
      }

      return static_cast<T *>(pv);
    }


    void deallocate(T * const p, const size_t /* n */) const 
    {
      Free(p);
    }

    template <typename U> T * allocate(const size_t n, const U * /* const hint */) const 
    {
      return allocate(n);
    }

  private:
    Allocator& operator=(const Allocator&);
  };

  
  template <typename T>
  void Free_T(T** ptr)
  {
    if (ptr && *ptr)
    {
      /*#ifdef _MSC_VER
      #pragma warning(push)
      #pragma warning(disable: 4100) // unreferenced formal parameter
      #endif*/
      (*ptr)->~T();
      /*#ifdef _MSC_VER
      #pragma warning(pop)
      #endif*/
      SLB::Free(*ptr);

      // To clearly identify deleted pointers:
      *ptr = 0L;
    }
  }

  template<typename T>
  void New_T(T **ptr)
  {
    if (ptr) *ptr = new (SLB::Malloc(sizeof(T))) T();
  }

}
#endif
