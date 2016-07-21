//
// Created by rthier on 2016.06.20..
//

#ifndef NFTSIMPLEPROJ_MATERIALDATA_H
#define NFTSIMPLEPROJ_MATERIALDATA_H

#include <vector>
#include <stdint.h>
#include <string>
#include "Material.h"
#include "TexturePreparationLibrary.h"
#include "GpuTexturePreparationLibrary.h"

namespace ObjMaster {
/**
 * This class contains everything a material is having, but add the handling of the associated data.
 * Basically the textures can be loaded / unloaded and via the provided texture loading library you
 * can even load/unload the texture data to the graphics card. So basically this contains everything
 * a material contains, but there is a two-level loading of textures (and other associated data):
 * - Load/unload into the main memory from files or sources via the TexturePreparationLibrary
 * - Loading/unloading into the video card memory using the provided TexturePreparationLibrary
 *
 * The template parameter should have the following static methods:
 * - void loadIntoMemory(const char* path, const char* textureFileName, std::vector<uint8_t> bitmap)
 * - unsigned int loadIntoGPU(std::vector<uint8_t> bitmap)
 * - void unloadFromGPU(unsigned int handle)
 * (unload method for unloading from the memory is not necessary of course)
 */
    class TextureDataHoldingMaterial : public Material {
    public:
        /** Defines the state of different texture holding states */
        enum TextureLoadState {
            /** Textures are *NOT* being held on the given holder */
            NOT_LOADED,
            /** Textures are being held on the given holder */
            LOADED
        };

        /**
         * Variable that holds the state of textures for this material in system memory.
         * When the textures of the material are preloaded into system ram, this becomes
         * "TextureDataHoldingMaterial::TextureLoadState::LOADED". When the material is loaded into
         * system ram, the data is prepared to be loaded onto the GPU. After the data is on the
         * GPU one can unload the bitmaps from the main memory and the GPU-loaded things should
         * still operate afterwards! This enables various ram-optimizations to take place.
         */
        TextureLoadState memoryHoldingState;
        /**
         * Variable that holds the state of textures for this material in gpu video memory.
         * When the textures of the material are preloaded into gpu video-ram, this becomes
         * "TextureDataHoldingMaterial::TextureLoadState::LOADED". When the material is loaded into
         * video ram, the data should be prepared to be rendered with the used graphics API and
         * the corresponding handles for texture binding should be filed. After the data is on the
         * GPU one can unload the bitmaps from the main memory and the GPU-loaded things should
         * still operate afterwards! This enables various ram-optimizations to take place. Even when
         * this variable is holding a LOADED value, there can be texture handles with zero values!
         */
        TextureLoadState gpuHoldingState;

        // Bitmap patterns of the loaded textures when loading into main memory
        // Rem.: vectors of empty size means there is no texture of that kind

        /** Possible ambient texture bitmap data - size=0 means no texture data */
        std::vector<uint8_t> tex_ka;
        /** Possible diffuse texture bitmap data - size=0 means no texture data */
        std::vector<uint8_t> tex_kd;
        /** Possible specular texture bitmap data - size=0 means no texture data */
        std::vector<uint8_t> tex_ks;
        /** Possible bump texture bitmap data - size=0 means no texture data */
        std::vector<uint8_t> tex_bump;

        // These are for storing handles when loading the texture onto the GPU
        // Rem.: the value of 0 means that the texture is not in use / uploaded to the GPU

        /** Possible ambient texture graphics card handle - value=0 means no GPU-loaded texture data */
        unsigned int tex_handle_ka;
        /** Possible diffuse texture graphics card handle - value=0 means no GPU-loaded texture data */
        unsigned int tex_handle_kd;
        /** Possible specular texture graphics card handle - value=0 means no GPU-loaded texture data */
        unsigned int tex_handle_ks;
        /** Possible bump texture graphics card handle - value=0 means no GPU-loaded texture data */
        unsigned int tex_handle_bump;


        /** Create a material with possible texture data using the given texture path */
        TextureDataHoldingMaterial(std::string materialName,
                                   std::vector<std::string> descriptorLineFields);

        TextureDataHoldingMaterial() {}

	void loadTexturesIntoMemory(const char *texturePath, const TexturePreparationLibrary &textureLib);
	void loadTexturesIntoGPU(const GpuTexturePreparationLibrary &textureLib);
	void unloadTexturesFromMemory();
	void unloadTexturesFromGPU(const GpuTexturePreparationLibrary &textureLib);
    };
}

#endif //NFTSIMPLEPROJ_MATERIALDATA_H
