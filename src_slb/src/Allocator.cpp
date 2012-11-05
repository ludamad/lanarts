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

#include <SLB/Allocator.hpp>
#include <SLB/Debug.hpp>
#include <cstdlib>

namespace
{
  static SLB::MallocFn s_mallocFn = NULL;
  static SLB::FreeFn s_freeFn = NULL;
}

namespace SLB
{
  SLB_EXPORT void SetMemoryManagement(MallocFn mallocFn, FreeFn freeFn)
  {
    s_mallocFn = mallocFn;
    s_freeFn = freeFn;
  }

  SLB_EXPORT void* Malloc(size_t sz)
  {
    SLB_DEBUG_CALL;
    void *result = 0;
    if (s_mallocFn)
    {
      result = (s_mallocFn)(sz);
      SLB_DEBUG(100, "Allocating memory (allocator=%p) : %lu bytes -> %p", (void*)s_mallocFn, sz, result);
    }
    else
    {
      result = malloc(sz);
      SLB_DEBUG(100, "Allocating memory (allocator='default') : %lu bytes -> %p", sz, result);
    }

    return result;
  }

  SLB_EXPORT void Free(void* ptr)
  {
    SLB_DEBUG_CALL;
    if (s_freeFn)
    {
      SLB_DEBUG(100, "Deallocating memory (deallocator=%p) : ptr %p", (void*) s_freeFn, ptr);
      (s_freeFn)(ptr);
    }
    else
    {
      SLB_DEBUG(100, "Deallocating memory (deallocator='default') : ptr %p", ptr);
      free(ptr);
    }
  }
}
