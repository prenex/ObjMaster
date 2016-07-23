#ifndef OM_TEXTURE_H
#define OM_TEXTURE_H

namespace ObjMaster {
	/** Represents a texture */
	struct Texture {
		/**
		 * The bitmap data of this texture when in the system memory.
		 *
		 * It can have a size of zero in case the texture is not loaded
		 * into the main RAM at this time.
		 */
		std::vector<uint8_t> bitmap;
		/**
		 * The handle for the texture when it is on the GPU.
		 * 
		 * It has the special value of 0 when the texture is not loaded
		 * onto the GPU unit.
		 */
		unsigned int handle;
		/** Width in pixels */
		int width;
		/** Height in pixels */
		int heigth;
		/** How many BYTES a pixel is represented on */
		int bytepp;
	};
}

#endif
