/*
 * gl_extensions.h:
 *  Managing uniform interface to OpenGL extensions
 */

#ifndef GL_EXTENSIONS_H_
#define GL_EXTENSIONS_H_

// Attempts to enable/disable vsync, returns false if it is unable to do so
bool gl_set_vsync(bool state);

#endif /* GL_EXTENSIONS_H_ */
