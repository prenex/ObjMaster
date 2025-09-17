#ifndef PRINT_GL_ERROR_H
#define PRINT_GL_ERROR_H
/* Needs something like: #include <GLES2/gl2.h> */

/** Logs possible GL error - only if -DDEBUG_EXTRA is set */
static inline void printGlError(std::string where) {
#ifdef DEBUG_EXTRA
	GLenum err = glGetError();
	if(err != GL_NO_ERROR) {
	OMLOGE((where + " - glError: 0x%x").c_str(), err);
	}
#endif
}

#endif /* PRINT_GL_ERROR_H */
