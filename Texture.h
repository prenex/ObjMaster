#ifndef OM_TEXTURE_H
#define OM_TEXTURE_H

#include <cstdint>

namespace ObjMaster {
	/** Represents a texture */
	struct Texture {
		Texture() {}
		Texture(
			std::vector<uint8_t> t_bitmap,
			unsigned int t_handle,
			int t_width,
			int t_heigth,
			int t_bytepp
		) : bitmap(t_bitmap),
			handle(t_handle),
			width(t_width),
			heigth(t_heigth),
			bytepp(t_bytepp)
		{}

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
		unsigned int handle{};
		/** Width in pixels */
		int width{};
		/** Height in pixels */
		int heigth{};
		/** How many BYTES a pixel is represented on */
		int bytepp{};

		/** Unload bitmap data from main memory - metadata and handle stays as is! */
		void unloadBitmapFromMemory() {
			bitmap.clear();
		}
	};
}

#endif
