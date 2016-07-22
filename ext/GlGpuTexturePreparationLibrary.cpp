#include "GlGpuTexturePreparationLibrary.h"
#include "../objmasterlog.h"

// TODO: We should somehow make it possible to use texture units as the different textures (bump map, specular map etc.) need to be there to the shaders the same time!
// TODO: I guess we should make it possible to batch-load a lot of textures and free a lot at once for better performance...
namespace ObjMasterExt {
		/** Load the given bitmap onto the GPU texture memory and return 'handle' */
		unsigned int GlGpuTexturePreparationLibrary::loadIntoGPU(const std::vector<uint8_t> &bitmap) const {
			// Generate texture object
			unsigned int handle;
			glGenTextures(1, &handle);
			glBindTexture(GL_TEXTURE_2D, handle);
			// Set the texture image of that texture object
			// Use bilinear filtering with mip-mapping so that minification is not a problem
			// this is necessary because the textures are really high-res ones!
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// Load the texture data onto the GPU
			// TODO: we should store size, bpp and such things when loading into memory lol! :-)
			// FIXME !!!! Temporal code!
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(bitmap[0]));

			OMLOGD("GlGpuTexturePreparationLibrary - glTexImage2D loaded %d bytes to the GPU!", (int)bitmap.size());
#ifdef DEBUG
if(bitmap.size() > 0) {
	for(int i = 4; i < bitmap.size(); i+=4) {
		// Just to ensure access works...
		OMLOGD("RGBA: %d %d %d %d", (&(bitmap[0]))[i-4], (&(bitmap[0]))[i-3], (&(bitmap[0]))[i-2], (&(bitmap[0]))[i-1]);
	}
}
#endif
			// Return a handle to the uploaded texture
			return handle;
		}

		/** Unload the texture data bound to the given handle (glDeleteTexture()) */
		void GlGpuTexturePreparationLibrary::unloadFromGPU(unsigned int handle) const {
			// Delete the texture for the given handle
			glDeleteTextures(1, &handle);
		}
}
