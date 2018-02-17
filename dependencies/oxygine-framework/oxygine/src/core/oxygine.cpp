#include "oxygine.h"
#include <stdio.h>
#include "log.h"
#include "VideoDriver.h"
#include "res/CreateResourceContext.h"
#include "res/Resources.h"


#include "res/ResBuffer.h"
#include "res/ResFontBM.h"
#include "res/ResAtlas.h"
#include "Image.h"
#include "PointerState.h"
#include "Input.h"

#include "DebugActor.h"
#include "Stage.h"
#include "KeyEvent.h"
#include "STDFileSystem.h"
#include "res/ResStarlingAtlas.h"


#include "Mutex.h"

#include "gl/oxgl.h"
#include "winnie_alloc/winnie_alloc.h"
#include "ThreadDispatcher.h"
#include "PostProcess.h"
#include "TextField.h"
#include "InputText.h"

#ifdef __S3E__
#include "s3e.h"
#include "IwGL.h"
#include "s3eSurface.h"
#include "s3eDebug.h"
#include "s3eTimer.h"
#include "s3ePointer.h"
#include "s3eOSExec.h"
#endif


#include "gl/VideoDriverGLES20.h"
#include "STDMaterial.h"


#ifdef EMSCRIPTEN
#include <sys/time.h>
#include <emscripten.h>
//#include <SDL.h>
//#include <SDL_compat.h>
#include <SDL_events.h>
#elif __ANDROID__
#include "core/android/jniUtils.h"
#elif __APPLE__
#include <TargetConditionals.h>
#include "ios/ios.h"
#endif

#include "pthread.h"

#ifdef OXYGINE_SDL
extern "C"
{
#include "SDL.h"
#include "SDL_video.h"
};

#include <stdio.h>
//#include "SDL_syswm.h"
#include "SDL_timer.h"

#endif


#ifdef EMSCRIPTEN
#   define HANDLE_FOCUS_LOST 0

void emscStackTrace()
{
    EM_ASM(
    {
        console.log(new Error().stack);
        /*
        try {
            throw new Error();
        } catch (e) {
            print(e.stack);
        }
        */
    });
}

#elif !SDL_VIDEO_OPENGL
#   define HANDLE_FOCUS_LOST 1
#else
#   define HANDLE_FOCUS_LOST 0
#endif

#define LOST_RESET_CONTEXT 0


namespace oxygine
{
    static ThreadDispatcher _threadMessages;
    static ThreadDispatcher _uiMessages;
    Mutex mutexAlloc;

    bool _useTouchAPI = false;

#ifdef OXYGINE_SDL
    static SDL_Window* _window = 0;
    static SDL_GLContext _context = 0;
#endif

    static core::init_desc desc;
    Point _qtFixedSize(0, 0);

    spEventDispatcher _dispatcher;

    static pthread_t _mainThread;

    namespace key
    {
        void update();
    }

    namespace core
    {
        static bool active = true;
        static bool focus = true;

        void focusLost()
        {
            if (!LOST_RESET_CONTEXT)
                return;

#if OXYGINE_SDL
            log::messageln("focus lost");
            SDL_GL_DeleteContext(_context);
            _context = 0;
#endif

        }

        void focusAcquired()
        {
            if (!LOST_RESET_CONTEXT)
                return;

#if OXYGINE_SDL
            log::messageln("lost context");
            if (!_context)
            {
                _context = SDL_GL_CreateContext(_window);
                initGLExtensions(SDL_GL_GetProcAddress);
            }
#endif
        }

        bool isMainThread()
        {
#ifdef OX_NO_MT
            return true;
#else
            return pthread_equal(_mainThread, pthread_self()) != 0;
#endif
        }

#ifdef EMSCRIPTEN
        void SDL_handleEvent(SDL_Event& event, bool& done);
        int SDL_eventsHandler(void*, SDL_Event* e)
        {
            bool done = false;
            SDL_handleEvent(*e, done);
            return 0;
        }
#endif



        void updateUIMessages()
        {
            ThreadDispatcher::peekMessage msg;
            while (_uiMessages.peek(msg, true)) {}
        }

        void init2();

        void init0()
        {
            if (!_dispatcher)
                _dispatcher = new EventDispatcher;
        }

        int init(init_desc* desc_ptr)
        {
            std::string t;

#ifdef OX_DEBUG
            t += "OX_DEBUG ";
#endif

#ifdef NDEBUG
            t += "NDEBUG ";
#endif

#ifdef _DEBUG
            t += "_DEBUG ";
#endif

#ifdef DEBUG
            t += "DEBUG ";
#endif

            log::messageln("build settings %s", t.c_str());

            init0();

            log::messageln("initialize oxygine");
            if (desc_ptr)
                desc = *desc_ptr;


            focus = true;
            active = true;

#ifdef __S3E__
            log::messageln("S3E build");
            if (!IwGLInit())
            {
                s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "IwGLInit failed");
                return;
            }
            //init_ext();

            int width = IwGLGetInt(IW_GL_WIDTH);
            int height = IwGLGetInt(IW_GL_HEIGHT);

            log::messageln("Screen BPP  : %d", s3eSurfaceGetInt(S3E_SURFACE_PIXEL_TYPE) & S3E_SURFACE_PIXEL_SIZE_MASK);
            log::messageln("Screen Size : %dx%d", width, height);
            log::messageln("Vendor      : %s", (const char*)glGetString(GL_VENDOR));
            log::messageln("Renderer    : %s", (const char*)glGetString(GL_RENDERER));

            const char* version = (const char*)glGetString(GL_VERSION);
            log::messageln("Version    : %s", version);

            s3ePointerUpdate();
            if (s3ePointerGetInt(S3E_POINTER_MULTI_TOUCH_AVAILABLE))
            {
                s3ePointerRegister(S3E_POINTER_TOUCH_EVENT, &pointerTouchEvent, 0);
                s3ePointerRegister(S3E_POINTER_TOUCH_MOTION_EVENT, &pointerTouchMotionEvent, 0);
            }
            else
            {
                s3ePointerRegister(S3E_POINTER_BUTTON_EVENT, &pointerEvent, 0);
                s3ePointerRegister(S3E_POINTER_MOTION_EVENT, &pointerMotionEvent, 0);
            }

            s3eDeviceRegister(S3E_DEVICE_UNPAUSE, applicationUnPause, 0);
            s3eDeviceRegister(S3E_DEVICE_PAUSE, applicationPause, 0);

#elif OXYGINE_SDL

            log::messageln("SDL build");


            SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, desc.allow_screensaver ? "1" : "0");

            SDL_Init(SDL_INIT_VIDEO);


            if (desc.mode24bpp)
            {
                SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            }
            else
            {
                SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
            }

            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
            //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
            //SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

            SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);

            if (desc.force_gles)
            {
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
            }

            SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

            int flags = SDL_WINDOW_OPENGL;

#if TARGET_OS_IPHONE
            flags |= SDL_WINDOW_SHOWN;
            flags |= SDL_WINDOW_BORDERLESS;
            flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#else
            if(desc.show_window)
                flags |= SDL_WINDOW_SHOWN;

            if(desc.borderless)
                flags |= SDL_WINDOW_BORDERLESS;

            if(desc.resizable)
                flags |= SDL_WINDOW_RESIZABLE;
#endif

            //SDL_DisplayMode mode;
            //SDL_GetCurrentDisplayMode(0, &mode);
            //log::messageln("display mode: %d %d", mode.w, mode.h);

            if (desc.w == -1 && desc.h == -1)
            {
                desc.w = 960;
                desc.h = 640;
            }

            if (desc.fullscreen)
                flags |= SDL_WINDOW_FULLSCREEN;


            Event ev(EVENT_PRECREATEWINDOW);
            _dispatcher->dispatchEvent(&ev);


#if TARGET_OS_IPHONE
            //ios bug workaround
            //flags &= ~SDL_WINDOW_FULLSCREEN;
#endif

#if TARGET_OS_IPHONE || defined(__ANDROID__)
            desc.w = -1;
            desc.h = -1;
#endif

            log::messageln("creating window %d %d", desc.w, desc.h);

            _window = SDL_CreateWindow(desc.title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, desc.w, desc.h, flags);

            if (!_window)
            {
                log::error("can't create window: %s", SDL_GetError());
#ifdef __ANDROID__
                jniRestartApp();
#endif
                return -1;
            }
            _context = SDL_GL_CreateContext(_window);
            if (!_context)
            {
                log::error("can't create gl context: %s", SDL_GetError());
#ifdef __ANDROID__
                jniRestartApp();
#endif
                return -1;
            }

            SDL_GL_SetSwapInterval(desc.vsync ? 1 : 0);

#ifdef EMSCRIPTEN
            SDL_SetEventFilter(SDL_eventsHandler, 0);

            int v = EM_ASM_INT(
            {
                var p = navigator.platform;
                if (p == 'iPad' || p == 'iPhone' || p == 'iPod')
                    return 1;
                return 0;
            }, 0);

            if (v)
                _useTouchAPI = true;

#endif

#if __ANDROID__ || TARGET_OS_IPHONE
            //if (SDL_GetNumTouchDevices() > 0)
            _useTouchAPI = true;
#endif

#endif

            init2();

            return 1;
        }


        void init2()
        {

#ifndef OX_NO_MT
            _mainThread = pthread_self();
#endif

            if (!_dispatcher)
                _dispatcher = new EventDispatcher;

#ifdef OXYGINE_EDITOR
            //setlocale(LC_ALL, "POSIX");
#endif

            file::init(desc.companyName, desc.appName);

#ifdef OXYGINE_SDL
            int missing = initGLExtensions(SDL_GL_GetProcAddress);
#endif

            Point size = getDisplaySize();
            log::messageln("display size: %d %d", size.x, size.y);


#if __S3E__
            int glversion = s3eGLGetInt(S3E_GL_VERSION);
            int major_gl = glversion >> 8;
            OX_ASSERT(major_gl == 2 && "gl version should be 2");
            IVideoDriver::instance = new VideoDriverGLES20();
#endif

            IVideoDriver::instance = new VideoDriverGLES20();


            CHECKGL();

            IVideoDriver::instance->setDefaultSettings();

            CHECKGL();



            STDRenderer::initialize();

            Resources::registerResourceType(ResAtlas::create, "atlas");
            Resources::registerResourceType(ResBuffer::create, "buffer");
            Resources::registerResourceType(ResFontBM::create, "font");
            Resources::registerResourceType(ResFontBM::createBM, "bmfc_font");
            Resources::registerResourceType(ResFontBM::createSD, "sdfont");
            Resources::registerResourceType(ResStarlingAtlas::create, "starling");

            STDRenderer::instance = new STDRenderer;
            STDMaterial::instance = new STDMaterial(STDRenderer::instance);

            CHECKGL();

#ifdef OX_DEBUG
#ifndef OXYGINE_EDITOR
            DebugActor::initialize();
            TextField::setDefaultFont(DebugActor::resSystem->getResFont("system"));
#endif
#endif
            log::messageln("oxygine initialized");
        }

#if OXYGINE_SDL
        Vector2 convertTouch(SDL_Event& ev)
        {
            Point size = getDisplaySize();
            return Vector2(ev.tfinger.x * size.x, ev.tfinger.y * size.y);
        }
#endif



        bool isActive()
        {
            return active;
        }

        bool hasFocus()
        {
            return focus;
        }

        spEventDispatcher getDispatcher()
        {
            return _dispatcher;
        }


#if OXYGINE_SDL
        SDL_GLContext getGLContext()
        {
            return _context;
        }

        SDL_Window* getWindow()
        {
            return _window;
        }
#endif

        void reset()
        {
            log::messageln("core::reset()");
            clearPostProcessItems();
            Restorable::releaseAll();
            PostProcess::freeShaders();
            STDRenderer::reset();
            IVideoDriver::instance->reset();
            log::messageln("core::reset() done");
        }

        void restore()
        {
            log::messageln("core::restore()");
            IVideoDriver::instance->restore();
            STDRenderer::restore();
            Restorable::restoreAll();
            log::messageln("core::restore() done");
        }

        bool isReady2Render()
        {
            return STDRenderer::isReady();
        }

        bool  beginRendering(window w)
        {

#ifdef OXYGINE_SDL
            SDL_Window* wnd = w;
            if (!wnd)
            {
                if (!focus)
                {
                    //log::messageln("!focus");
                    return false;
                }

                wnd = _window;
            }
            SDL_GL_MakeCurrent(wnd, _context);
#endif

            CHECKGL();

            bool ready = STDRenderer::isReady();
            if (ready)
            {
                IVideoDriver::_stats.start = getTimeMS();
                updatePortProcessItems();
            }
            else
            {
                log::messageln("!ready");
            }

            return ready;
        }

        void swapDisplayBuffers(window w)
        {
#if OXYGINE_SDL
            if (!_context)
                return;
#endif

            CHECKGL();
#if __S3E__
            IwGLSwapBuffers();
#elif OXYGINE_SDL
            SDL_Window* wnd = w;
            if (!wnd)
                wnd = _window;
            if (_context)
            {
                int status = SDL_GL_MakeCurrent(wnd, _context);
                if (status)
                {
                    log::error("SDL_GL_MakeCurrent(): %s", SDL_GetError());
                }
                SDL_GL_SwapWindow(wnd);
            }
#endif
            CHECKGL();

            IVideoDriver::_stats.duration = getTimeMS() - IVideoDriver::_stats.start;
            //sleep(1000/50);
        }

#ifdef __S3E__
#elif OXYGINE_EDITOR
#else
        spStage getStageByWindow(Uint32 windowID)
        {
            SDL_Window* wnd = SDL_GetWindowFromID(windowID);
            if (!wnd)
                return getStage();

            spStage stage = Stage::getStageFromWindow(wnd);
            if (!stage)
                return getStage();

            return stage;
        }

        void SDL_handleEvent(SDL_Event& event, bool& done)
        {
            Input* input = &Input::instance;




            Event ev(EVENT_SYSTEM);
            ev.userData = &event;
            _dispatcher->dispatchEvent(&ev);

            if (!ev.stopsPropagation)
                switch (event.type)
                {
                    case SDL_QUIT:
                        log::messageln("SDL_QUIT");
                        done = true;
                        break;
                    case SDL_WINDOWEVENT:
                    {
                        /*
                        if (event.window.event == SDL_WINDOWEVENT_ENTER)
                        active = false;
                        if (event.window.event == SDL_WINDOWEVENT_LEAVE)
                        active = true;
                        */

                        log::messageln("SDL_WINDOWEVENT %d", (int)event.window.event);

                        if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
                            active = false;
                        if (event.window.event == SDL_WINDOWEVENT_RESTORED)
                            active = true;

                        bool newFocus = focus;
                        if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                            newFocus = false;
                        if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                            newFocus = true;
#ifdef __ANDROID__
                        //if (event.window.event == SDL_WINDOWEVENT_ENTER)
                        //   newFocus = true;
#endif

                        if (focus != newFocus)
                        {
#if HANDLE_FOCUS_LOST
                            focus = newFocus;

                            if (focus)
                                focusAcquired();

                            log::messageln("focus: %d", (int)focus);
                            Event ev(focus ? Stage::ACTIVATE : Stage::DEACTIVATE);

                            spStage stage = getStageByWindow(event.window.windowID);
                            if (stage)
                                stage->dispatchEvent(&ev);

                            if (!focus)
                                focusLost();
#endif
                        }
                        //log::messageln("SDL_SYSWMEVENT %d", (int)event.window.event);
                        break;
                    }
                    case SDL_MOUSEWHEEL:
                        input->sendPointerWheelEvent(getStageByWindow(event.window.windowID), Vector2((float)event.wheel.x, (float)event.wheel.y), &input->_pointerMouse);
                        break;
                    case SDL_KEYDOWN:
                    {
                        KeyEvent ev(KeyEvent::KEY_DOWN, &event.key);
                        getStageByWindow(event.window.windowID)->dispatchEvent(&ev);
                    } break;
                    case SDL_KEYUP:
                    {
                        KeyEvent ev(KeyEvent::KEY_UP, &event.key);
                        getStageByWindow(event.window.windowID)->dispatchEvent(&ev);
                    } break;

                    case SDL_MOUSEMOTION:
                        if (!_useTouchAPI)
                            input->sendPointerMotionEvent(getStageByWindow(event.window.windowID), (float)event.motion.x, (float)event.motion.y, 1.0f, &input->_pointerMouse);
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                    {
                        if (!_useTouchAPI)
                        {
                            MouseButton b = MouseButton_Left;
                            switch (event.button.button)
                            {
                                case 1: b = MouseButton_Left; break;
                                case 2: b = MouseButton_Middle; break;
                                case 3: b = MouseButton_Right; break;
                            }

                            input->sendPointerButtonEvent(getStageByWindow(event.window.windowID), b, (float)event.button.x, (float)event.button.y, 1.0f,
                                                          event.type == SDL_MOUSEBUTTONDOWN ? TouchEvent::TOUCH_DOWN : TouchEvent::TOUCH_UP, &input->_pointerMouse);
                        }
                    }
                    break;
                    case SDL_FINGERMOTION:
                    {
                        if (_useTouchAPI)
                        {
                            //log::messageln("SDL_FINGERMOTION");
                            Vector2 pos = convertTouch(event);
                            PointerState* ps = input->getTouchByID((int64_t)event.tfinger.fingerId);
                            if (ps)
                                input->sendPointerMotionEvent(getStageByWindow(event.window.windowID),
                                                              pos.x, pos.y, event.tfinger.pressure, ps);
                        }
                    }

                    break;
                    case SDL_FINGERDOWN:
                    case SDL_FINGERUP:
                    {
                        if (_useTouchAPI)
                        {
                            //log::messageln("SDL_FINGER");
                            Vector2 pos = convertTouch(event);
                            PointerState* ps = input->getTouchByID((int64_t)event.tfinger.fingerId);
                            if (ps)
                                input->sendPointerButtonEvent(getStageByWindow(event.window.windowID),
                                                              MouseButton_Touch,
                                                              pos.x, pos.y, event.tfinger.pressure,
                                                              event.type == SDL_FINGERDOWN ? TouchEvent::TOUCH_DOWN : TouchEvent::TOUCH_UP,
                                                              ps);
                        }
                    }
                    break;
                }

        }
#endif

        bool update()
        {
#ifndef OXYGINE_EDITOR
            key::update();
#endif

            timeMS duration = IVideoDriver::_stats.duration;
            IVideoDriver::_stats = IVideoDriver::Stats();
            IVideoDriver::_stats.duration = duration;

            ThreadDispatcher::peekMessage msg;
            while (_threadMessages.peek(msg, true)) {}

#ifdef __S3E__

            s3eDeviceYield(0);
            s3eKeyboardUpdate();
            s3ePointerUpdate();

            bool done = false;

            if (s3eDeviceCheckQuitRequest())
                done = true;

            if (s3eKeyboardGetState(s3eKeyEsc) & S3E_KEY_STATE_PRESSED)
                done = true;

            return done;
#endif


#if OXYGINE_SDL

            //log::messageln("update");

            bool done = false;
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
#if !EMSCRIPTEN //emscripten build handles events from EventsFilter
                SDL_handleEvent(event, done);
#endif
            }

            return done;
#endif

            log::warning("update not implemented");
            return true;
        }

        void release()
        {
            log::messageln("core::release");

#ifndef OXYGINE_EDITOR
            InputText::stopAnyInput();
#endif

            _threadMessages.clear();
            _uiMessages.clear();

            clearPostProcessItems();
            PostProcess::freeShaders();

            Event ev(EVENT_EXIT);
            _dispatcher->dispatchEvent(&ev);

            Material::setCurrent(0);
            STDRenderer::release();
            delete STDMaterial::instance;
            STDMaterial::instance = 0;

            delete IVideoDriver::instance;
            IVideoDriver::instance = 0;

            Input::instance.cleanup();


            DebugActor::release();

            if (Stage::instance)
                Stage::instance->cleanup();
            Stage::instance = 0;
            file::free();


            Resources::unregisterResourceType("atlas");
            Resources::unregisterResourceType("buffer");
            Resources::unregisterResourceType("font");
            Resources::unregisterResourceType("bmfc_font");
            Resources::unregisterResourceType("sdfont");
            Resources::unregisterResourceType("starling");



#if OXYGINE_SDL
            SDL_GL_DeleteContext(_context);
            SDL_DestroyWindow(_window);
            SDL_Quit();
#endif

            _dispatcher = 0;
        }

        void execute(const char* str)
        {
#ifdef __S3E__

            s3eOSExecExecute(str, false);
#elif OXYGINE_EDITOR
#elif __ANDROID__
            jniBrowse(str);
#elif EMSCRIPTEN
            EM_ASM_INT(
            {
                var url = Pointer_stringify($0);
                window.open(url, '_blank');
            }, str);
#elif __APPLE__ && TARGET_OS_IPHONE
            iosNavigate(str);
#else
            OX_ASSERT(!"execute not implemented");
#endif
        }

        std::string getPackage()
        {
#ifdef __ANDROID__
            return jniGetPackage();
#else
            return "unknown";
#endif
        }

        void requestQuit()
        {
            log::messageln("requestQuit");
#ifdef __S3E__
            s3eDeviceRequestQuit();
#elif OXYGINE_SDL
            SDL_Event ev;
            ev.type = SDL_QUIT;
            SDL_PushEvent(&ev);
#endif
        }

        ThreadDispatcher& getMainThreadDispatcher()
        {
            return _threadMessages;
        }

        ThreadDispatcher& getUiThreadMessages()
        {
            return _uiMessages;
        }


        Point getDisplaySize()
        {
#if __S3E__
            int width = IwGLGetInt(IW_GL_WIDTH);
            int height = IwGLGetInt(IW_GL_HEIGHT);

            int orient = s3eSurfaceGetInt(S3E_SURFACE_DEVICE_ORIENTATION_LOCK);
            if (height > width)
            {
                if (orient == S3E_SURFACE_LANDSCAPE || orient == S3E_SURFACE_LANDSCAPE_FIXED)
                    std::swap(width, height);
            }
            else
            {
                if (orient == S3E_SURFACE_PORTRAIT || orient == S3E_SURFACE_PORTRAIT_FIXED)
                    std::swap(width, height);
            }

            return Point(width, height);
#elif OXYGINE_EDITOR
            return _qtFixedSize;
#elif OXYGINE_SDL
            int w = 0;
            int h = 0;

            SDL_GL_GetDrawableSize(_window, &w, &h);
            return Point(w, h);
#else
            log::warning("getDisplaySize not implemented");
            return Point(0, 0);
#endif
        }
    }

    void handleErrorPolicy(error_policy ep, const char* format, ...)
    {
        va_list args;
        va_start(args, format);

        switch (ep)
        {
            case ep_show_error:
                log::error_va(format, args);
                OX_ASSERT_NL(!"handleErrorPolicy error.");
                break;
            case ep_show_warning:
                log::warning_va(format, args);
                break;
            case ep_ignore_error:
                break;
            default:
                OX_ASSERT(!"not implemented");
        }

        va_end(args);
    }

    bool    isNetworkAvailable()
    {
#if __S3E__
        return s3eSocketGetInt(S3E_SOCKET_NETWORK_AVAILABLE) == 1;
#elif __ANDROID__
        return jniIsNetworkAvailable();
#endif
        return true;
    }

    int64 getFreeSpace(const char* fullpath /*= 0*/)
    {
#ifdef __ANDROID__
        if (fullpath)
            return jniGetFreeSpace(fullpath);
        return jniGetFreeSpace(file::wfs().getFullPath("").c_str());
#elif __APPLE__ && TARGET_OS_IPHONE
        return iosGetFreeDiskspace();
#endif

        return std::numeric_limits<int64>::max();
    }

    std::string     getLanguage()
    {
#ifdef __ANDROID__
        return jniGetLanguage();
#endif
        return "en";
    }

    void    sleep(timeMS time)
    {
#if __S3E__
        s3eDeviceYield(time);
#elif OXYGINE_SDL
        SDL_Delay(time);
#else
        log::warning("sleep not implemented");
#endif
    }
}
