#ifndef PRINT_GL_ERROR_H
#define PRINT_GL_ERROR_H

static void printGlError(std::string where) {
	GLenum err = glGetError();
	if(err != GL_NO_ERROR) {
	OMLOGE((where + " - glError: 0x%x").c_str(), err);
	}
}

#endif /* PRINT_GL_ERROR_H */
