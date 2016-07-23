#ifndef STB_IMG_TEXTUREPREPARATIONLIBRARY_H
#define STB_IMG_TEXTUREPREPARATIONLIBRARY_H

#include <stdint.h>
#include <string>
#include <vector>
#include "TexturePreparationLibrary.h"
#include "Texture.h"

namespace ObjMaster {
    /** Uses stb_image.h */
    class StbImgTexturePreparationLibrary : public TexturePreparationLibrary {
	public:
		Texture loadIntoMemory(const char *path,
					   const char *textureFileName) const;
    };
}

#endif // STB_IMG_NOPTEXTUREPREPARATIONLIBRARY_H
