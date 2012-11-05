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

#ifndef __SLB_CONFIG__
#define __SLB_CONFIG__

// Enable to compile thread safety support
#ifndef SLB_THREAD_SAFE
  #define SLB_THREAD_SAFE 0
#endif

// Enable to support exceptions, otherwise asserts will be used
#ifndef SLB_USE_EXCEPTIONS
  #define SLB_USE_EXCEPTIONS 0
#endif

// To compile SLB as a dynamic library
// You also need to define SLB_LIBRARY macro when compiling SLB's cpp's
#ifndef SLB_DYNAMIC_LIBRARY
  #define SLB_DYNAMIC_LIBRARY 0
#endif

// use SLB's internal lua (latest lua version available)
// or use lua from your own project.
#ifndef SLB_EXTERNAL_LUA
  #define SLB_EXTERNAL_LUA 0
#endif

// If you want to speed up things a little define
// this as 0 and no check will be done.
#ifndef SLB_CHECK_FUNC_ARGUMENTS
  #define SLB_CHECK_FUNC_ARGUMENTS 1
#endif

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
  #define SLB_WINDOWS
#endif


// -- EXCEPTIONS -----------------------------------------------------------------
#if SLB_USE_EXCEPTIONS
  #define SLB_CRITICAL_ERROR(...) /*nothing*/
  #define SLB_THROW(...) throw __VA_ARGS__
#else
  #include <stdlib.h>
  #define SLB_CRITICAL_ERROR(msg) \
    {fprintf(stderr, "SLB Critical Error (%s:%d) -> %s", __FILE__, __LINE__, msg); \
    exit(129);}
  #define SLB_THROW(...) /*nothing*/
#endif // SLB_USE_EXCEPTIONS
// ------------------------------------------------------------------------------


// -- DEBUG -----------------------------------------------------------------
#ifndef SLB_DEBUG_OUTPUT
  #define SLB_DEBUG_OUTPUT stderr
#endif

// you can redefine the function to be used to debug, should have a 
// printf-like interface.
#ifndef SLB_DEBUG_FUNC
  #include <cstdio>
  #include <cstring>
  #define SLB_DEBUG_FUNC(...) fprintf(SLB_DEBUG_OUTPUT, __VA_ARGS__);
#endif

#ifndef SLB_DEBUG_LEVEL
  #define SLB_DEBUG_LEVEL 0
#endif
// --------------------------------------------------------------------------

// Specific MSC pragma's
#if defined(_MSC_VER)
#pragma warning( disable: 4251 )
#pragma warning( disable: 4290 )
#pragma warning( disable: 4127 ) // constant expressions
#endif

#endif
