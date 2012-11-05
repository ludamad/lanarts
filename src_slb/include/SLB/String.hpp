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

#ifndef __SLB_STRING__
#define __SLB_STRING__

#include "Allocator.hpp"
#include "Export.hpp"
#include <string>


namespace SLB
{
#if 1
  typedef std::basic_string< char, std::char_traits<char>, Allocator<char> > String;
#else
  class SLB_EXPORT String : public std::basic_string< char, std::char_traits<char>, Allocator<char> > 
  {
  public:
    typedef std::basic_string< char, std::char_traits<char>, Allocator<char> > Base;
    String() {}
    String(const String& s) : Base(s.c_str()) {}
    String(const char* s) : Base(s) {}
    String(const Base& b) : Base(b) {}
    String(const std::string& s) : Base(s.c_str()) {}

    String& operator=(const String& s) { if (&s != this) { Base::operator=(s.c_str()); } return *this; }
    String& operator=(const char* c) { Base::operator=(c); return *this; }

    bool operator==(const char *c)   const { return ( (*(Base*)this) == c); }
    bool operator==(const String& s) const { return ( (*(Base*)this) == s); }
    bool operator<(const char *c)    const { return ( (*(Base*)this) < c);  }
    bool operator<(const String &s)  const { return ( (*(Base*)this) < s);  }
  };
#endif

}

#endif
