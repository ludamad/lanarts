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



#ifndef __SLB_INSTANCE__
#define __SLB_INSTANCE__

#include "ref_ptr.hpp"
#include "Manager.hpp"
#include "Debug.hpp"
#include "Export.hpp"
#include "Allocator.hpp"

#define SLB_INSTANCE \
  virtual const void *memoryRawPointer() const { return this; }

namespace SLB {

  class SLB_EXPORT InstanceBase
  {
  public:
    enum Type
    {
      I_Invalid    = 0,
      I_UserFlag_0 = 1<<0,
      I_UserFlag_1 = 1<<1,
      I_UserFlag_2 = 1<<2,
      I_UserFlag_3 = 1<<3,
      I_UserFlag_4 = 1<<4,
      I_UserFlag_5 = 1<<5,
      I_UserFlag_6 = 1<<6,
      I_UserFlag_7 = 1<<7,
      //-----------------------
      I_UserMask   = 0xFF,
      //-----------------------
      I_MustFreeMem   = 1<<17,
      I_Reference     = 1<<18,
      I_Pointer       = 1<<19,
      I_Const_Pointer = 1<<20,
    };

    // functions to override:
    virtual void* get_ptr() = 0;
    virtual const void* get_const_ptr() = 0;
    virtual const void *memoryRawPointer() const = 0;

    // constructor:
    InstanceBase(Type , ClassInfo *c);

    ClassInfo *getClass() { return _class.get(); }

    size_t getFlags() const { return _flags; }
    size_t getUserFlags() const { return _flags & I_UserMask; }
    void addUserFlags(size_t flags) { _flags = _flags | (flags & I_UserMask); }
    void setUSerFlags(size_t flags) { _flags = (flags & I_UserMask); }

    bool mustFreeMem() const { return (_flags & I_MustFreeMem) != 0; }
    bool isConst() const     { return (_flags & I_Const_Pointer) != 0; }
    bool isPointer() const   { return (_flags & I_Pointer) || (_flags & I_Const_Pointer); }
    bool isReference() const { return (_flags & I_Reference) != 0; }
    virtual ~InstanceBase();

    // This is called if the memory comes from a constructor inside the script
    // You probably would never need to call this manually
    void setMustFreeMemFlag() { _flags = _flags | I_MustFreeMem; }

  protected:
    size_t _flags;
    ref_ptr<ClassInfo> _class;
  };

  namespace Instance {

    struct Default {
      template<class T>
      class Implementation : public virtual InstanceBase
      {
      public:
        // constructor from a pointer 
        Implementation(ClassInfo *ci, T* ptr) : InstanceBase( I_Pointer,ci ), _ptr(ptr)
        {
        }
        // constructor from const pointer
        Implementation(ClassInfo *ci, const T *ptr ) : InstanceBase( I_Const_Pointer, ci), _const_ptr(ptr)
        {
        }

        // constructor from reference
        Implementation(ClassInfo *ci, T &ref ) : InstanceBase( I_Reference, ci ), _ptr( &ref )
        {
        }

        // copy constructor,  
        Implementation(ClassInfo *ci, const T &ref) : InstanceBase( I_MustFreeMem, ci ), _ptr( 0L )
        {
          _ptr = new (Malloc(sizeof(T))) T(ref);
        }

        virtual ~Implementation() { if (mustFreeMem()) Free_T(&_ptr); }

        void* get_ptr() { return (isConst())? 0L : _ptr; }
        const void* get_const_ptr() { return _const_ptr; }
      protected:
        union {
          T *_ptr;
          const T *_const_ptr;
        };
        SLB_INSTANCE;
      };
    };


    struct NoCopy
    {
      template<class T>
      class Implementation : public virtual InstanceBase
      {
      public:
        Implementation(ClassInfo *ci, T* ptr) : InstanceBase( I_Pointer, ci ), _ptr(ptr)
        {
        }
        // constructor from const pointer
        Implementation(ClassInfo *ci, const T *ptr ) : InstanceBase( I_Const_Pointer, ci), _const_ptr(ptr)
        {
        }

        // constructor from reference
        Implementation(ClassInfo *ci, T &ref ) : InstanceBase( I_Reference, ci ), _ptr( &ref )
        {
        }

        // copy constructor,  
        Implementation(ClassInfo *ci, const T &ref) : InstanceBase( I_Invalid, ci ), _ptr( 0L )
        {
        }

        virtual ~Implementation() { if (mustFreeMem()) Free_T(&_ptr); }

        void* get_ptr() { return (isConst())? 0L : _ptr; }
        const void* get_const_ptr() { return _const_ptr; }
      protected:
        union {
          T *_ptr;
          const T *_const_ptr;
        };
        SLB_INSTANCE;
      };
    };

    struct NoCopyNoDestroy 
    {
      template<class T>
      class Implementation : public virtual InstanceBase
      {
      public:
        // constructor form a pointer 
        Implementation(ClassInfo *ci, T* ptr) : InstanceBase( I_Pointer, ci ), _ptr(ptr)
        {
        }
        // constructor from const pointer
        Implementation(ClassInfo *ci, const T *ptr ) : InstanceBase( I_Const_Pointer, ci), _const_ptr(ptr)
        {
        }

        // constructor from reference
        Implementation(ClassInfo *ci, T &ref ) : InstanceBase( I_Reference, ci ), _ptr( &ref )
        {
        }

        // copy constructor,  
        Implementation(ClassInfo *ci, const T &) : InstanceBase( I_Invalid, ci ), _ptr( 0L )
        {
        }

        virtual ~Implementation() {}

        void* get_ptr() { return (isConst())? 0L : _ptr; }
        const void* get_const_ptr() { return _const_ptr; }
      protected:
        union {
          T *_ptr;
          const T *_const_ptr;
        };
        SLB_INSTANCE;
      };
    };

    template<template <class> class T_SmartPtr>
    struct SmartPtr 
    {
      template<class T>
      class Implementation : public virtual InstanceBase
      {
      public:
        Implementation(ClassInfo *ci, T* ptr) : InstanceBase( I_Pointer, ci ), _sm_ptr(ptr)
        {
          _const_ptr = &(*_sm_ptr);
        }
        Implementation(ClassInfo *ci, const T *ptr ) : InstanceBase( I_Const_Pointer, ci), _const_ptr(ptr)
        {
        }
        // What should we do with references and smart pointers?
        Implementation(ClassInfo *ci, T &ref ) : InstanceBase( I_Reference, ci ), _sm_ptr( &ref )
        {
          _const_ptr = &(*_sm_ptr);
        }

        // copy constructor,  
        Implementation(ClassInfo *ci, const T &ref) : InstanceBase( I_MustFreeMem, ci ), _sm_ptr( 0L ), _const_ptr(0)
        {
          _sm_ptr = new (Malloc(sizeof(T))) T( ref );
          _const_ptr = &(*_sm_ptr);
        }

        virtual ~Implementation() {}

        void* get_ptr() { return &(*_sm_ptr); }
        const void* get_const_ptr() { return _const_ptr; }

      protected:
        T_SmartPtr<T> _sm_ptr;
        const T *_const_ptr;
        SLB_INSTANCE;
      };
    };

    template<template <class> class T_SmartPtr>
    struct SmartPtrNoCopy
    {
      template<class T>
      class Implementation : public virtual InstanceBase
      {
      public:
        Implementation(ClassInfo *ci, T* ptr) : InstanceBase( I_Pointer, ci ), _sm_ptr(ptr)
        {
          _const_ptr = &(*_sm_ptr);
        }
        Implementation(ClassInfo *ci, const T *ptr ) : InstanceBase( I_Const_Pointer,ci), _const_ptr(ptr)
        {
        }

        // What should we do with references and smart pointers?
        Implementation(ClassInfo *ci, T &ref ) : InstanceBase( I_Reference, ci ), _sm_ptr( &ref )
        {
          _const_ptr = &(*_sm_ptr);
        }

        // copy constructor,  
        Implementation(ClassInfo *ci, const T &ref) : InstanceBase( I_Invalid, ci ), _sm_ptr( 0L ), _const_ptr(&ref)
        {
        }

        virtual ~Implementation() {}

        void* get_ptr() { return &(*_sm_ptr); }
        const void* get_const_ptr() { return _const_ptr; }

      protected:
        T_SmartPtr<T> _sm_ptr;
        const T *_const_ptr;
        SLB_INSTANCE;
      };
    };

    template<template <class> class T_SmartPtr>
    struct SmartPtrSharedCopy
    {
      template<class T>
      class Implementation : public virtual InstanceBase
      {
      public:
        Implementation(ClassInfo *ci, T* ptr) : InstanceBase( I_Pointer, ci ), _sm_ptr(ptr)
        {
          _const_ptr = &(*_sm_ptr);
        }
        Implementation(ClassInfo *ci, const T *ptr ) : InstanceBase( I_Const_Pointer, ci), _const_ptr(ptr)
        {
        }

        Implementation(ClassInfo *ci, T &ref ) : InstanceBase( I_Reference, ci ), _sm_ptr( &ref )
        {
          _const_ptr = &(*_sm_ptr);
        }

        // copy constructor,  
        Implementation(ClassInfo *ci, const T &ref) : InstanceBase( I_Invalid, ci ), _sm_ptr( 0L ), _const_ptr(&ref)
        {
          T *obj = const_cast<T*>(&ref);
          _sm_ptr = obj;
        }

        virtual ~Implementation() {}

        void* get_ptr() { return &(*_sm_ptr); }
        const void* get_const_ptr() { return _const_ptr; }

      protected:
        T_SmartPtr<T> _sm_ptr;
        const T *_const_ptr;
        SLB_INSTANCE;
      };
    };

  
  } // end of Instance namespace


  struct SLB_EXPORT InstanceFactory
  {
    // create an Instance from a reference
    virtual InstanceBase *create_ref(Manager *m, void *ref) = 0;
    // create an Instance from a pointer
    virtual InstanceBase *create_ptr(Manager *m, void *ptr) = 0;
    // create an Instance from a const pointer
    virtual InstanceBase *create_const_ptr(Manager *m, const void *const_ptr) = 0;
    // create an Instance with copy
    virtual InstanceBase *create_copy(Manager *m, const void *ptr) = 0;

    virtual ~InstanceFactory();
  };

  template<class T, class TInstance >
  struct InstanceFactoryAdapter : public InstanceFactory
  {
    virtual InstanceBase *create_ref(Manager *m, void *v_ref)
    {
      T &ref = *reinterpret_cast<T*>(v_ref);
      ClassInfo *ci = m->getClass(_TIW(T));
      return new (Malloc(sizeof(TInstance))) TInstance(ci, ref);
    }

    virtual InstanceBase *create_ptr(Manager *m, void *v_ptr)
    {
      T *ptr = reinterpret_cast<T*>(v_ptr);
      ClassInfo *ci = m->getClass(_TIW(T));
      return new (Malloc(sizeof(TInstance))) TInstance(ci, ptr);
    }

    virtual InstanceBase *create_const_ptr(Manager *m, const void *v_ptr)
    {
      const T *const_ptr = reinterpret_cast<const T*>(v_ptr);
      ClassInfo *ci = m->getClass(_TIW(T));
      return new (Malloc(sizeof(TInstance))) TInstance(ci, const_ptr);
    }

    virtual InstanceBase *create_copy(Manager *m, const void *v_ptr)
    {
      const T &const_ref = *reinterpret_cast<const T*>(v_ptr);
      ClassInfo *ci = m->getClass(_TIW(T));
      return new (Malloc(sizeof(TInstance))) TInstance(ci, const_ref);
    }

    virtual ~InstanceFactoryAdapter() {}
  };
}


#endif
