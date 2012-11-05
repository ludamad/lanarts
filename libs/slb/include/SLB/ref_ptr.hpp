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



#ifndef __SLB_REF_PTR__
#define __SLB_REF_PTR__

namespace SLB {

  template<class T>
  class ref_ptr
  {
  public:
        typedef T element_type;

        ref_ptr() :_ptr(0L) {}
        ref_ptr(T* t):_ptr(t) { if (_ptr) _ptr->ref(); }
        ref_ptr(const ref_ptr& rp):_ptr(rp._ptr)  { if (_ptr) _ptr->ref(); }
        ~ref_ptr() { if (_ptr) _ptr->unref(); _ptr=0L; }

        inline ref_ptr& operator = (const ref_ptr& rp)
        {
            if (_ptr==rp._ptr) return *this;
            T* tmp_ptr = _ptr;
            _ptr = rp._ptr;
            if (_ptr) _ptr->ref();
            if (tmp_ptr) tmp_ptr->unref();
            return *this;
        }

        inline ref_ptr& operator = (T* ptr)
        {
            if (_ptr==ptr) return *this;
            T* tmp_ptr = _ptr;
            _ptr = ptr;
            if (_ptr) _ptr->ref();
            if (tmp_ptr) tmp_ptr->unref();
            return *this;
        }

        // comparison operators for ref_ptr.
        inline bool operator == (const ref_ptr& rp) const { return (_ptr==rp._ptr); }
        inline bool operator != (const ref_ptr& rp) const { return (_ptr!=rp._ptr); }
        inline bool operator < (const ref_ptr& rp) const { return (_ptr<rp._ptr); }
        inline bool operator > (const ref_ptr& rp) const { return (_ptr>rp._ptr); }

        // comparison operator for const T*.
        inline bool operator == (const T* ptr) const { return (_ptr==ptr); }
        inline bool operator != (const T* ptr) const { return (_ptr!=ptr); }
        inline bool operator < (const T* ptr) const { return (_ptr<ptr); }
        inline bool operator > (const T* ptr) const { return (_ptr>ptr); }

        inline T& operator*() { return *_ptr; }
        inline const T& operator*() const { return *_ptr; }
        inline T* operator->() { return _ptr; }
        inline const T* operator->() const { return _ptr; }
    inline bool operator!() const { return _ptr==0L; }
    inline bool valid() const { return _ptr!=0L; }
        inline T* get() { return _ptr; }
        inline const T* get() const { return _ptr; }
    inline T* release() { T* tmp=_ptr; if (_ptr) _ptr->unref_nodelete(); _ptr=0; return tmp;}

  private:
        T* _ptr;
  };

}


#endif
