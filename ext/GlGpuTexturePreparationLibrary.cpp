#include "GlGpuTexturePreparationLibrary.h"

// TODO: We should somehow make it possible to use texture units as the different textures (bump map, specular map etc.) need to be there to the shaders the same time!
namespace ObjMasterExt {
		/** Load the given bitmap onto the GPU texture memory and return 'handle' */
		unsigned int GlGpuTexturePreparationLibrary::loadIntoGPU(const std::vector<uint8_t> &bitmap) const {
			// Generate texture object
			unsigned int handle;
			glGenTextures(1, &handle);
			glBindTexture(GL_TEXTURE_2D, handle);
			// Set the texture image of that texture object
			// TODO: we should store size, bpp and such things when loading into memory lol! :-)
			// FIXME !!!! Temporal code!
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(bitmap[0]));
			return handle;
		}

		/** Unload the texture data bound to the given handle (glDeleteTexture()) */
		void GlGpuTexturePreparationLibrary::unloadFromGPU(unsigned int handle) const {
			glDeleteTextures(1, &handle);
		}
}
