//
// Created by rthier on 2016.06.20..
//

#ifndef NFTSIMPLEPROJ_MATERIALDATA_H
#define NFTSIMPLEPROJ_MATERIALDATA_H

#include <vector>
#include <stdint.h>
#include <string>
#include "Material.h"

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

        /** Load all textures into the main memory
         * - with different paths, one can provide different texture files for same material texture
         *   entries! This add some extra freedom for developers to implement various schemes...
         * - if memoryHoldingState == TextureDataHoldingMaterial::TextureLoadState::LOADED" then
         *   this operation first unload the earlier data with unloadTexturesFromMemory()!
         */
        template<class TexturePreparationLibrary>
        void loadTexturesIntoMemory(const char *texturePath) {
            if(memoryHoldingState == TextureLoadState::LOADED){
                unloadTexturesFromMemory();
            }
            // Load data in when that texture applies according to the fields of the read material
            if(enabledFields[Material::F_MAP_KA]) {
                loadIntoMemory<TexturePreparationLibrary>(texturePath, map_ka.c_str(), tex_ka);        // ka
            }
            if(enabledFields[Material::F_MAP_KD]) {
                loadIntoMemory<TexturePreparationLibrary>(texturePath, map_kd.c_str(), tex_kd);        // kd
            }
            if(enabledFields[Material::F_MAP_KS]) {
                loadIntoMemory<TexturePreparationLibrary>(texturePath, map_ks.c_str(), tex_ks);        // ks
            }
            if(enabledFields[Material::F_MAP_BUMP]) {
                loadIntoMemory<TexturePreparationLibrary>(texturePath, map_bump.c_str(), tex_bump);    // bump
            }
            memoryHoldingState = TextureLoadState::LOADED;
        }

        /**
         * Load all textures into the GPU-memory (prepare for rendering)
         * - This method sets zero handles for those textures that are not having loaded bitmaps in
         *   system ram.
         * - if gpuHoldingState == TextureDataHoldingMaterial::TextureLoadState::LOADED" then
         *   this operation first unload the earlier data with unloadTexturesFromGPU()!
         */
        template<class TexturePreparationLibrary>
        void loadTexturesIntoGPU() {
            if(gpuHoldingState == TextureLoadState::LOADED) {
                unloadTexturesFromGPU<TexturePreparationLibrary>();
            }
            // Load those textures into the GPU which have some data in the memory
            if(!tex_ka.empty()) { tex_handle_ka = loadIntoGPU<TexturePreparationLibrary>(tex_ka); }
            else {tex_handle_ka = 0; }
            if(!tex_kd.empty()) { tex_handle_kd = loadIntoGPU<TexturePreparationLibrary>(tex_kd); }
            else {tex_handle_kd = 0; }
            if(!tex_ks.empty()) { tex_handle_ks = loadIntoGPU<TexturePreparationLibrary>(tex_ks); }
            else {tex_handle_ks = 0; }
            if(!tex_bump.empty()) { tex_handle_bump = loadIntoGPU<TexturePreparationLibrary>(tex_bump); }
            else {tex_handle_bump = 0; }
            gpuHoldingState = TextureLoadState::LOADED;
        }

        /** Unload all textures from the main memory */
        void unloadTexturesFromMemory() {
            // Clear texture data in memory
            tex_ka.clear();
            tex_kd.clear();
            tex_ks.clear();
            tex_bump.clear();
            memoryHoldingState = TextureLoadState::NOT_LOADED;
        }

        /** Unload all textures from the GPU-memory. Unload is only called on non-zero handles */
        template<class TexturePreparationLibrary>
        void unloadTexturesFromGPU() {
            // Only call the unload on those handles that are non-zero
            if(tex_handle_ka != 0) { unloadFromGPU<TexturePreparationLibrary>(tex_handle_ka); tex_handle_ka = 0; }
            if(tex_handle_kd != 0) { unloadFromGPU<TexturePreparationLibrary>(tex_handle_kd); tex_handle_kd = 0; }
            if(tex_handle_ks != 0) { unloadFromGPU<TexturePreparationLibrary>(tex_handle_ks); tex_handle_ks = 0; }
            if(tex_handle_bump != 0) { unloadFromGPU<TexturePreparationLibrary>(tex_handle_bump); tex_handle_bump = 0; }
            gpuHoldingState = TextureLoadState::NOT_LOADED;
        }

        /** Create a material with possible texture data using the given texture path */
        TextureDataHoldingMaterial(std::string materialName,
                                   std::vector<std::string> descriptorLineFields);

        TextureDataHoldingMaterial() {}
    private:

        // TEMPLATE OPERATION'S IMPLEMENTATIONS
        // ------------------------------------
        // For the resolution of the template parameter's operations, we provide implementations here
        // so that "real" dependent code can be in the *.cpp file

        // check this part in case you want to implement a preparation library!!!
        template<class TexturePreparationLibrary>
        void loadIntoMemory(const char *path, const char *textureFileName,
                            std::vector<uint8_t> bitmap) {
            TexturePreparationLibrary::loadIntoMemory(path, textureFileName, bitmap);
        }

        template<class TexturePreparationLibrary>
        unsigned int loadIntoGPU(std::vector <uint8_t> bitmap) {
            return TexturePreparationLibrary::loadIntoGPU(bitmap);
        }

        template<class TexturePreparationLibrary>
        void unloadFromGPU(unsigned int handle) {
            TexturePreparationLibrary::unloadFromGPU(handle);
        }
    };
}

#endif //NFTSIMPLEPROJ_MATERIALDATA_H
