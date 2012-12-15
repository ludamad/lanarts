/*
 * gl_extensions.cpp:
 *  Managing uniform interface to OpenGL extensions
 */

#include <cstdio>
#include <cstring>

#include <string>

#include <GL/glu.h>

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

//TODO: Test this on windows!

#include <GL/wglext.h>

static bool wgl_check_extension(const std::string& extension) {
    // this is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    // determine pointer to wglGetExtensionsStringEXT function
    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");

    return query_ext_string(extension, _wglGetExtensionsString());
}
// Attempts to enable/disable vsync, returns false if it is unable to do so
bool gl_set_vsync(bool state) {
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
	if (!wgl_check_extension("WGL_EXT_swap_control")) return false;

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) LogGetProcAddress("wglSwapIntervalEXT");
    wglSwapIntervalEXT(state);
    return true;
}

#else // not windows

#ifdef __linux__

#include <GL/glx.h>

static bool glx_check_extension(const std::string& extension) {
	Display* dpy = glXGetCurrentDisplay();
	int screen = DefaultScreen(dpy);

	std::string extensions = (const char*) glXQueryExtensionsString(dpy,
			screen);
	return query_ext_string(extension, extensions);
}

// Attempts to enable/disable vsync, returns false if it is unable to do so
bool gl_set_vsync(bool state) {
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
