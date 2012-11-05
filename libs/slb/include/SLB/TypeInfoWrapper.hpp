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



#ifndef __SLB_TYPEINFO_WRAPPER__
#define __SLB_TYPEINFO_WRAPPER__

#include <typeinfo>

namespace SLB {

  template<class T>
  class TypeID
  {
  public:
    static const char *Name();
    static unsigned long Hash();
  private:
    TypeID();
    static unsigned long hash_;
  };

  template<class T>
  unsigned long TypeID<T>::hash_ = 0;

  template<class T>
  const char *TypeID<T>::Name()
  {
    #if defined(_MSC_VER)
      return __FUNCTION__ ;
    #elif defined(__GNUC__) or defined(__SNC__)
      return __PRETTY_FUNCTION__;
    #else
      return __func__
    #endif
  }
  
  template<class T>
  inline unsigned long TypeID<T>::Hash()
  {
    if (hash_ == 0)
    {
      const char *name = Name();
      static const unsigned long hbits = 31;
      static const unsigned long hprime = (unsigned long) 16777619;
      static const unsigned long hmod = (unsigned long) 1 << hbits;
      while (*name != '\0')
      {
        hash_ = (hash_*hprime) % hmod;
        hash_ = hash_ ^ ((unsigned long) *name);
        name++;
      }
    }
    return hash_;
  }
  

  class TypeInfoWrapper
  {
  public:

    TypeInfoWrapper() :
      _ID(0), _name("Invalid")
    {
    }

    TypeInfoWrapper(unsigned long hash, const char *name) :
      _ID(hash), _name(name)
    {
    
    }

    unsigned long type() const { return _ID; }

    const char *name() const { return _name; }

    bool operator<(const TypeInfoWrapper &o) const
    {
      return _ID < o._ID;
    }

    bool operator==(const TypeInfoWrapper &o) const
    {
      return _ID == o._ID;
    }

    bool operator!=(const TypeInfoWrapper &o) const
    {
      return _ID != o._ID;
    }
    
  private:
    unsigned long _ID;
    const char *_name;
  };

#define _TIW(x) ::SLB::TypeInfoWrapper(::SLB::TypeID<x>::Hash(), ::SLB::TypeID<x>::Name())

} // end of SLB Namespace

#endif
