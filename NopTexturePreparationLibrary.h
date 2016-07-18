//
// Created by rthier on 2016.06.21..
//

#ifndef NFTSIMPLEPROJ_NOPTEXTUREPREPARATIONLIBRARY_H
#define NFTSIMPLEPROJ_NOPTEXTUREPREPARATIONLIBRARY_H

#include<stdint.h>
#include<string>
#include<vector>

namespace ObjMaster {
    /** This texture preparation library does not prepare and load textures at all! */
    class NopTexturePreparationLibrary {
        static void loadIntoMemory(const char *path,
                                   const char *textureFileName,
                                   std::vector<uint8_t> bitmap) {
            // We don't do a thing here
        }

        static unsigned int loadIntoGPU(std::vector<uint8_t> bitmap) {
            // 0 is a special value indicating the data is not on the GPU
            return 0;
        }

        static void unloadFromGPU(unsigned int handle) {
            // Not necessary as we never load it onto the GPU anyways
        }
    };
}

#endif //NFTSIMPLEPROJ_NOPTEXTUREPREPARATIONLIBRARY_H
