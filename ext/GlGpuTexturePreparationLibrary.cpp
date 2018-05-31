#include "GlGpuTexturePreparationLibrary.h"
#include "../objmasterlog.h"

// TODO: We should somehow make it possible to use texture units as the different textures (bump map, specular map etc.) need to be there to the shaders the same time!
// TODO: I guess we should make it possible to batch-load a lot of textures and free a lot at once for better performance...
namespace ObjMasterExt {
		/** Load the given bitmap onto the GPU texture memory and return 'handle' */
		void GlGpuTexturePreparationLibrary::loadIntoGPU(ObjMaster::Texture &t) const {
			if(t.bitmap.size() > 0) {
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
				// TODO: Currently supporting only RGBA and RGB
				auto mode = t.bytepp == 4 ? GL_RGBA : GL_RGB;
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.width, t.heigth, 0, mode, GL_UNSIGNED_BYTE, &(t.bitmap[0]));
				// TODO: mip-mapping?

				OMLOGD("GlGpuTexturePreparationLibrary - glTexImage2D loaded %d bytes to the GPU to handle %u!", (int)t.bitmap.size(), handle);
#ifdef DEBUG
if(bitmap.size() > 0) {
	for(int i = 4; i < bitmap.size(); i+=4) {
		// Just to ensure access works...
		OMLOGD("RGBA: %d %d %d %d", (&(bitmap[0]))[i-4], (&(bitmap[0]))[i-3], (&(bitmap[0]))[i-2], (&(bitmap[0]))[i-1]);
	}
}
#endif
				// Set the handle to the uploaded texture handle we have generated with GL
				t.handle = handle;
			}
		}

		/** Unload the texture data bound to the given handle (glDeleteTexture()) */
		void GlGpuTexturePreparationLibrary::unloadFromGPU(ObjMaster::Texture &t) const {
			if(t.handle != 0) {
				// Delete the texture for the given handle
				// We can safely do the cast here as the
				// texture handle must have been loaded with us!
				// and in OpenGL it is just a number not a pointer.
				// In other graphics APIs (like DirectX) they use
				// pointers instead of handles so the library uses
				// a type that is surely big enough to hold that too!
				glDeleteTextures(1, (const GLuint*) &t.handle);
				// A number of zero means that there is no texture!
				t.handle = 0;
			}
		}
}
