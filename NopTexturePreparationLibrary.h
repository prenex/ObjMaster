//
// Created by rthier on 2016.06.21..
//

#ifndef NOPTEXTUREPREPARATIONLIBRARY_H
#define NOPTEXTUREPREPARATIONLIBRARY_H

#include <stdint.h>
#include <string>
#include <vector>
#include "TexturePreparationLibrary.h"
#include "GpuTexturePreparationLibrary.h"

namespace ObjMaster {
    /** This texture preparation library does not prepare and load textures at all! */
    class NopTexturePreparationLibrary : public TexturePreparationLibrary, public GpuTexturePreparationLibrary {
	public:
		Texture loadIntoMemory(const char *path,
					   const char *textureFileName) const {
		    // We don't do a thing here, just return empty vector
		    return Texture {
			std::vector<uint8_t>(),
			0,
			0,
			0,
			0
		    };
		}

		unsigned int loadIntoGPU(const std::vector<uint8_t> &bitmap) const {
		    // 0 is a special value indicating the data is not on the GPU
		    return 0;
		}

		void unloadFromGPU(unsigned int handle) const {
		    // Not necessary as we never load it onto the GPU anyways
		}
    };
}

#endif //NOPTEXTUREPREPARATIONLIBRARY_H
