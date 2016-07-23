#include "StbImgTexturePreparationLibrary.h"
// TODO: Ensure that this is in the good place for defining the implementation "only once" according to the specs.
#define STB_IMAGE_IMPLEMENTATION
#include "deps/stb_image.h"
#include "objmasterlog.h"

namespace ObjMaster {
	/** Load an image into the memory with stb_image.h */
	Texture StbImgTexturePreparationLibrary::loadIntoMemory(const char *path,
					   const char *textureFileName) const {
		int width, heigth;
		int bytePerPixel;
		std::string pStr(path);
		// Load the bitmap with stb_image.h - last param (the 0) means auto-detection of bpp
		uint8_t* image = stbi_load((pStr + textureFileName).c_str(), &width, &heigth, &bytePerPixel, 0);

		// I need the array length to create the vector with its "range constructor" appropriately
		int len = width * heigth * bytePerPixel;

		// Create a vector with the range constructor
		// that has the copy of the 
		std::vector<uint8_t> bitmap(image, image + len);

		// Mirror onto the horizontal midline (from top to bottom)
		// this is needed as the shaders need thee 
		for(int y = 0; y < heigth / 2; ++y) {
			for(int x = 0; x < width * bytePerPixel; ++x) {
				uint8_t tmp = bitmap[y*width*bytePerPixel + x];
				bitmap[y*width*bytePerPixel + x] = bitmap[(heigth-y-1)*width*bytePerPixel + x];
				bitmap[(heigth-y-1)*width*bytePerPixel + x] = tmp;
			}
		}

		// free the original data array (we have our own copy from now)
		stbi_image_free(image);

		// Log texture load event
		OMLOGI("Texture (%s%s:%dx%d -- %d bytes) is loaded into the main memory with stb_image!", 
				path, 
				textureFileName,
				width,
				heigth,
				(int)bitmap.size());

		// Return our own copy in the vector. The whole lib of ours use
		// vectors instead of bare pointers and arrays as much as we can
		// so this copy is needed to adhere our earlier code. The return
		// value optimisations should make return copy go fast too.
		return Texture {
			bitmap,
			0, // handle = 0 as this is not loaded to the GPU
			width,
			heigth,
			bytePerPixel
		};
	}
}
