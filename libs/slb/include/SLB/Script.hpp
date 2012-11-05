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



#ifndef __SLB_SCRIPT__
#define __SLB_SCRIPT__

#include "lua.hpp" 
#include "PushGet.hpp"
#include "Type.hpp"
#include<stdexcept>

namespace SLB {

  class ErrorHandler; // #include <SLB/Error.hpp>
  
  class SLB_EXPORT Script
  {  
  public:
    typedef void (*PrintCallback)(
        Script *,     // Script that produced the print call
        const char *, // string to print
        size_t        // size of the string to print
    );

    explicit Script(Manager *m = Manager::defaultManager());
    virtual ~Script();

    // changes if the created lua_state will load default libraries when
    // it is created. This change only takes effect when the lua_State is 
    // created (when getState() method is called the first time). You can 
    // always call resetState() to make sure the next created state will
    // have or wont have default libraries loaded
    void loadDefaultLibs(bool b) { _loadDefaultLibs = b; }

    // Tries to load-and-execute the given file, it can throw an exception
    // If SLB was compiled with Exception support or can exit the program
    // with an error code. See safeDoFile for a safe call that doesn't fail.
    void doFile(const char *filename) SLB_THROW((std::exception));

    // Executes the content of the given file, returns true if the call
    // was successful, false otherwise. It never throws, or exits the program,
    // If you need error control use a custom ErrorHandler
    bool safeDoFile(const char *filename);

    // Tries to load and execute the given chunk of code. This method can
    // throw exceptions or exit the program (depends on SLB_USE_EXCEPTIONS 
    // definition). If you ned a safe call that doesn't fail use safeDoString
    // method instead.
    void doString(
      const char *codeChunk,
      const char *where_hint ="[SLB]") SLB_THROW((std::exception));

    // Executs the given code chunk and returns true if successful, false
    // otherwise. It never throws, or exits the program, if you need error
    // control use a custom ErrorHandler.
    bool safeDoString(
      const char *codeChunk,
      const char *where_hint ="[SLB]");

    // closes the current state, and will create a new state on the next
    // getState() call.
    void resetState() { close(); }

     /* ************************* WARNING *********************************
      * Sometines you need to manually call Garbage Collector(GC), to be sure
      * that all objects are destroyed. This is mandatory when using smartPointers
      * ,be very carefull. GC operations are really expensive, avoid calling GC
      * too frequently.
      * ************************* WARNING *********************************/
    void callGC();

    /// Returns the number of Kb of memory used by the script
    size_t memUsage();

    /// Pass a new ErrorHandler, the error handler will be part of the object
    /// and it will be destroyed when the object is destroyed. 
    void setErrorHandler(ErrorHandler *h);

    /// Returns the manager that this Script is using
    Manager* getManager() { return _manager; }
    const Manager* getManager() const { return _manager; }

    template<class T>
    void set(const char *name, T value)
    { SLB::setGlobal<T>(getState(), value, name);}

    template<class T>
    T get(const char *name)
    { return SLB::getGlobal<T>(getState(), name); }

    static void* allocator(void *ud, void *ptr, size_t osize, size_t nsize);

    const char *getLastError() const { return _lastError.c_str(); }

    // Changes the print callback. This callback will be called by the internal
    // print function from the scripts in order t
    void setPrintCallback( PrintCallback );

  protected:
    virtual void onNewState(lua_State * /*L*/) {}
    virtual void onCloseState(lua_State * /*L*/) {}
    virtual void onGC(lua_State * /*L*/) {}
    void setAllocator(lua_Alloc f, void *ud = 0);

    lua_State* getState();
    void close(); // will close lua_state

  private:
    Script(const Script &s);
    Script& operator=(const Script&);
    static int PrintHook(lua_State *);

    Manager *_manager;
    lua_State *_lua_state;
    PrintCallback _printCallback;
    lua_Alloc _allocator;
    void*     _allocator_ud;
    std::string _lastError;
    ErrorHandler *_errorHandler;
    bool _loadDefaultLibs;
  };

}

#endif
