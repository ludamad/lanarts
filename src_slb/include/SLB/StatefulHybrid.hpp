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



#ifndef __SLB_STATEFUL_HYBRID__
#define __SLB_STATEFUL_HYBRID__

#include "Hybrid.hpp" 
#include "Script.hpp" 

namespace SLB {
  
  /* S -> Requires to have a method "getState" and "close" */
  template<class T, class S = SLB::Script>
  class StatefulHybrid : public S, public Hybrid< T >
  {  
  public:
    StatefulHybrid(Manager *m = Manager::defaultManager()) :
      S(m), Hybrid<T>(m)
    {
    }
    virtual ~StatefulHybrid() { S::close(); }

    virtual bool isAttached() const;
    virtual lua_State *getLuaState() { return S::getState(); }

  protected:
    virtual void onNewState(lua_State *L) { HybridBase::attach( L ); S::onNewState(L); }
    virtual void onCloseState(lua_State *L) { HybridBase::unAttach(); S::onCloseState(L); }

  };
  
  template<class T, class S >
  inline bool StatefulHybrid<T,S>::isAttached() const
  {
    //StatefulHybrids are always attached (but as we use a lazy attachment, here
    // we have to simulate and do the attachment, that means throw away constness)
    
    StatefulHybrid<T,S> *me = const_cast< StatefulHybrid<T,S>* >(this);
    me->getState(); // That's enought to ensure the attachment
    return true;
  }

}

#endif
