/*
 * gl_extensions.cpp:
 *  Managing uniform interface to OpenGL extensions
 */

#include <cstdio>
#include <cstring>

#include <string>

#include "gl_extensions.h"

static bool query_ext_string(const std::string& extension,
		const std::string& extensions) {
	size_t ind = extensions.find(extension);
	if (ind == std::string::npos) {
		return false;
	}

	// Lets be extra sure we arent matching a substring
	size_t end_ind = ind + extension.size();

	if (ind > 0 && extensions[ind - 1] != ' ') {
		return false; // part of another string
	} else if (end_ind < extensions.size() && extensions[end_ind] != ' ') {
		return false; // part of another string
	}

	return true;
}

#if (defined (WIN32) || defined (_WIN32))

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include "wglext.h"

// Attempts to enable/disable vsync, returns false if it is unable to do so
bool gl_set_vsync(bool state) {
	typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

	const char* extensions = (const char*)glGetString( GL_EXTENSIONS );
	//TODOL correctly query wglExtension string
	if (false && !query_ext_string("WGL_EXT_swap_control", extensions)) {
		return false;
	}

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

	if (!wglSwapIntervalEXT) {
		return false;
	}

	wglSwapIntervalEXT(state);

    return true;
}

#else // not windows

#ifdef __arm__
// Fallback, just fail!
bool gl_set_vsync(bool state) {
	return false;
}

#elif __linux__

#include <GL/glx.h>

static bool glx_check_extension(const std::string& extension) {
	Display* dpy = glXGetCurrentDisplay();
	int screen = DefaultScreen(dpy);

	std::string extensions = (const char*) glXQueryExtensionsString(dpy,
			screen);
	return query_ext_string(extension, extensions);
}

// Attempts to enable/disable vsync, returns false if it is unable to do so
bool gl_set_vsync(bool state) {return true;
	typedef void (*swap_intervalf)(int);
	swap_intervalf swapinterval;
    if (glx_check_extension("GLX_MESA_swap_control")) {
    	swapinterval = (swap_intervalf) glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");
    } else if (glx_check_extension("GLX_SGI_swap_control")) {
    	swapinterval = (swap_intervalf) glXGetProcAddress((const GLubyte*) "glXSwapIntervalSGI");
    } else {
    	return false;
    }

    swapinterval(state);

	return true;
}

#else // not linux

// Fallback, just fail!
bool gl_set_vsync(bool state) {
	return false;
}

#endif // not linux

#endif // not windows
