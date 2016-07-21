#include "StbImgTexturePreparationLibrary.h"
#include "deps/stb_image.h"

namespace ObjMaster {
	/** Load an image into the memory with stb_image.h */
	std::vector<uint8_t> StbImgTexturePreparationLibrary::loadIntoMemory(const char *path,
					   const char *textureFileName) const {
		// TODO load the bitmap with stb_image.h
		std::vector<uint8_t> bitmap;
		return bitmap;
	}
}
