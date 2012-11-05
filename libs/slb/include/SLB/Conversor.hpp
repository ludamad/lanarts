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



#ifndef __SLB_CONVERSOR__
#define __SLB_CONVERSOR__

namespace SLB {
  
  struct Conversor
  {
    virtual void* operator()(void* obj) = 0;
    virtual ~Conversor() {}
  };

  template<class B, class D>
  struct BaseToDerived : public Conversor
  {
    void *operator()(void* obj_raw)
    {
      B *obj = reinterpret_cast<B*>(obj_raw);
      return dynamic_cast<D*>(obj);
    }
  };

  template<class D, class B>
  struct DerivedToBase : public Conversor
  {
    void *operator()(void* obj_raw)
    {
      D *obj = reinterpret_cast<D*>(obj_raw);
      B *base_obj = obj; // D -> B
      return base_obj;
    }
  };

}

#endif
