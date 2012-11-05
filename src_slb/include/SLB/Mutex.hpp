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


#include "Export.hpp"

#ifndef __SLB_MUTEX__
#define __SLB_MUTEX__

#if SLB_THREAD_SAFE == 0
  namespace SLB { struct MutexData {}; }
#else // SLB_THREAD_SAFE
  // Win32 Mutex:
  #ifdef SLB_WINDOWS
    #include <windows.h>
    namespace SLB { typedef CRITICAL_SECTION MutexData; }
  #else // WIN32
  // Posix Mutex:
    #include <pthread.h>  
    namespace SLB { typedef pthread_mutex_t MutexData; }
  #endif
#endif //SLB_THREAD_SAFE

namespace SLB
{

  /// Multiplatform Mutex abstraction, needed to ensure thread safety of
  /// some algorithms. You can turn on Mutexes by compileing SLB with
  /// SLB_THREAD_SAFE

  struct Mutex
  {
    Mutex();
    ~Mutex();
    void lock();
    void unlock();
    bool trylock();
  private:
    MutexData _m;
  };

  struct CriticalSection
  {
    CriticalSection(Mutex *m) : _m(m)
    {
      if (_m) _m->lock();
    }
    ~CriticalSection()
    {
      if (_m) _m->unlock();
    }
  private:
    Mutex *_m;
  };

  struct ActiveWaitCriticalSection
  {
    ActiveWaitCriticalSection(Mutex *m) : _m(m)
    {
      if (_m) while(_m->trylock() == false) {/*try again*/};
    }
    ~ActiveWaitCriticalSection()
    {
      if (_m) _m->unlock();
    }
  private:
    Mutex *_m;
  };


#if SLB_THREAD_SAFE == 0
  inline Mutex::Mutex() {}
  inline Mutex::~Mutex() {}
  inline void Mutex::lock(){}
  inline void Mutex::unlock() {}
  inline bool Mutex::trylock() { return true; }
#else // SLB_THREAD_SAFE
#ifdef WIN32
  // Windows implementation...
  inline Mutex::Mutex()
  {
    InitializeCriticalSection(&_m);
  }
  inline Mutex::~Mutex()
  {
    DeleteCriticalSection(&_m);
  }
  inline void Mutex::lock()
  {
    EnterCriticalSection(&_m);
  }
  inline void Mutex::unlock()
  {
    LeaveCriticalSection(&_m);
  }
  inline bool Mutex::trylock()
  {
    return( TryEnterCriticalSection(&_m) != 0) ;
  }
#else
  // PTHREADS implementation...
  inline Mutex::Mutex()
  {
    pthread_mutex_init(&_m,0);
  }
  inline Mutex::~Mutex()
  {
    pthread_mutex_destroy(&_m);
  }
  inline void Mutex::lock()
  {
    pthread_mutex_lock(&_m);
  }
  inline void Mutex::unlock()
  {
    pthread_mutex_unlock(&_m);
  }
  inline bool Mutex::trylock()
  {
    return( pthread_mutex_trylock(&_m) == 0) ;
  }
#endif // WIN32


#endif // SLB_THREAD_SAFE


} // end of SLB's namespace

#endif
