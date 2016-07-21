#ifndef GL_GPU_TEXTURE_PREPARATION_LIBRARY_H
#define GL_GPU_TEXTURE_PREPARATION_LIBRARY_H

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <Glut/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#include "../GpuTexturePreparationLibrary.h"

namespace ObjMasterExt {

	class GlGpuTexturePreparationLibrary : public ObjMaster::GpuTexturePreparationLibrary {
	public:
		/** Load the given bitmap onto the GPU texture memory and return 'handle' */
		virtual unsigned int loadIntoGPU(const std::vector<uint8_t> &bitmap) const;
		/** Unload the texture data bound to the given handle (glDeleteTexture()) */
		virtual void unloadFromGPU(unsigned int handle) const;
	};
}

#endif
