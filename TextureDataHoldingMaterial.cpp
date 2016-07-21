//
// Created by rthier on 2016.06.20..
//

#include "TextureDataHoldingMaterial.h"

namespace ObjMaster {

	// Rem.: Most of the implementation is in the header file, because of templating...
	TextureDataHoldingMaterial::TextureDataHoldingMaterial(std::string materialName,
							   std::vector<std::string> descriptorLineFields)
	    : Material(materialName, descriptorLineFields) {
	}


	/** Load all textures into the main memory
	 * - with different paths, one can provide different texture files for same material texture
	 *   entries! This add some extra freedom for developers to implement various schemes...
	 * - if memoryHoldingState == TextureDataHoldingMaterial::TextureLoadState::LOADED" then
	 *   this operation first unload the earlier data with unloadTexturesFromMemory()!
	 */
	void TextureDataHoldingMaterial::loadTexturesIntoMemory(const char *texturePath, const TexturePreparationLibrary &textureLib) {
	    if(memoryHoldingState == TextureLoadState::LOADED){
		unloadTexturesFromMemory();
	    }
	    // Load data in when that texture applies according to the fields of the read material
	    if(enabledFields[Material::F_MAP_KA]) {
		    // TODO: remove this test bogusness
		    printf("kalap"); // FIXME: Only this is printed even though the model has both Ka and Kd!!!
		tex_ka = textureLib.loadIntoMemory(texturePath, map_ka.c_str());        // ka
	    }
	    if(enabledFields[Material::F_MAP_KD]) {
		    // TODO: remove this test bogusness - it seems that we do not find the diffuse texture because of some errors!!!!!
		    printf("kabat");
		tex_kd = textureLib.loadIntoMemory(texturePath, map_kd.c_str());        // kd
	    }
	    if(enabledFields[Material::F_MAP_KS]) {
		tex_ks = textureLib.loadIntoMemory(texturePath, map_ks.c_str());        // ks
	    }
	    if(enabledFields[Material::F_MAP_BUMP]) {
		tex_bump = textureLib.loadIntoMemory(texturePath, map_bump.c_str());    // bump
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
	void TextureDataHoldingMaterial::loadTexturesIntoGPU(const GpuTexturePreparationLibrary &textureLib) {
	    if(gpuHoldingState == TextureLoadState::LOADED) {
		unloadTexturesFromGPU(textureLib);
	    }
	    // Load those textures into the GPU which have some data in the memory
	    if(!tex_ka.empty()) { tex_handle_ka = textureLib.loadIntoGPU(tex_ka); }
	    else {tex_handle_ka = 0; }
	    if(!tex_kd.empty()) { tex_handle_kd = textureLib.loadIntoGPU(tex_kd); }
	    else {tex_handle_kd = 0; }
	    if(!tex_ks.empty()) { tex_handle_ks = textureLib.loadIntoGPU(tex_ks); }
	    else {tex_handle_ks = 0; }
	    if(!tex_bump.empty()) { tex_handle_bump = textureLib.loadIntoGPU(tex_bump); }
	    else {tex_handle_bump = 0; }
	    gpuHoldingState = TextureLoadState::LOADED;
	}

	/** Unload all textures from the main memory */
	void TextureDataHoldingMaterial::unloadTexturesFromMemory() {
	    // Clear texture data in memory
	    tex_ka.clear();
	    tex_kd.clear();
	    tex_ks.clear();
	    tex_bump.clear();
	    memoryHoldingState = TextureLoadState::NOT_LOADED;
	}

	/** Unload all textures from the GPU-memory. Unload is only called on non-zero handles */
	void TextureDataHoldingMaterial::unloadTexturesFromGPU(const GpuTexturePreparationLibrary &textureLib) {
	    // Only call the unload on those handles that are non-zero
	    if(tex_handle_ka != 0) { textureLib.unloadFromGPU(tex_handle_ka); tex_handle_ka = 0; }
	    if(tex_handle_kd != 0) { textureLib.unloadFromGPU(tex_handle_kd); tex_handle_kd = 0; }
	    if(tex_handle_ks != 0) { textureLib.unloadFromGPU(tex_handle_ks); tex_handle_ks = 0; }
	    if(tex_handle_bump != 0) { textureLib.unloadFromGPU(tex_handle_bump); tex_handle_bump = 0; }
	    gpuHoldingState = TextureLoadState::NOT_LOADED;
	}

}
