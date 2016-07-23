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
	 * - This operation rewrites meta-data of textures, but keeps any handles still non-zero!
	 */
	void TextureDataHoldingMaterial::loadTexturesIntoMemory(const char *texturePath, const TexturePreparationLibrary &textureLib) {
	    if(memoryHoldingState == TextureLoadState::LOADED){
		unloadTexturesFromMemory();
	    }
	    // Load data in when that texture applies according to the fields of the read material
	    if(enabledFields[Material::F_MAP_KA]) {
		    // TODO: remove this test bogusness
		    printf("kalap"); // FIXME: Only this is printed even though the model has both Ka and Kd!!!
		// Save possible handle
		unsigned int handleTmp = tex_ka.handle;
		// Load the texture with the texture lib. As this return a new object,
		// the handle will be zero after this operation
		tex_ka = textureLib.loadIntoMemory(texturePath, map_ka.c_str());        // ka
		// Preserve earlier handle
		tex_ka.handle = handleTmp;
	    }
	    if(enabledFields[Material::F_MAP_KD]) {
		    // TODO: remove this test bogusness - it seems that we do not find the diffuse texture because of some errors!!!!!
		    printf("kabat");
		// Save possible handle
		unsigned int handleTmp = tex_kd.handle;
		// Load the texture with the texture lib. As this return a new object,
		// the handle will be zero after this operation
		tex_kd = textureLib.loadIntoMemory(texturePath, map_kd.c_str());        // kd
		// Preserve earlier handle
		tex_kd.handle = handleTmp;
	    }
	    if(enabledFields[Material::F_MAP_KS]) {
		// Save possible handle
		unsigned int handleTmp = tex_ks.handle;
		// Load the texture with the texture lib. As this return a new object,
		// the handle will be zero after this operation
		tex_ks = textureLib.loadIntoMemory(texturePath, map_ks.c_str());        // ks
		// Preserve earlier handle
		tex_ks.handle = handleTmp;
	    }
	    if(enabledFields[Material::F_MAP_BUMP]) {
		// Save possible handle
		unsigned int handleTmp = tex_bump.handle;
		// Load the texture with the texture lib. As this return a new object,
		// the handle will be zero after this operation
		tex_bump = textureLib.loadIntoMemory(texturePath, map_bump.c_str());    // bump
		// Preserve earlier handle
		tex_bump.handle = handleTmp;
	    }
	    memoryHoldingState = TextureLoadState::LOADED;

	    OMLOGD("Texture state after loadTexturesIntoMemory:");
	    OMLOGD(" - tex_ka.bitmap.size()=%d", (int)tex_ka.bitmap.size());
	    OMLOGD(" - tex_kd.bitmap.size()=%d", (int)tex_kd.bitmap.size());
	    OMLOGD(" - tex_ks.bitmap.size()=%d", (int)tex_ks.bitmap.size());
	    OMLOGD(" - tex_bump.bitmap.size()=%d", (int)tex_bump.bitmap.size());
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
	    if(!tex_ka.bitmap.empty()) { textureLib.loadIntoGPU(tex_ka); }
	    else {tex_ka.handle = 0; }
	    if(!tex_kd.bitmap.empty()) { textureLib.loadIntoGPU(tex_kd); }
	    else {tex_kd.handle = 0; }
	    if(!tex_ks.bitmap.empty()) { textureLib.loadIntoGPU(tex_ks); }
	    else {tex_ks.handle = 0; }
	    if(!tex_bump.bitmap.empty()) { textureLib.loadIntoGPU(tex_bump); }
	    else {tex_bump.handle = 0; }
	    gpuHoldingState = TextureLoadState::LOADED;

	    OMLOGD("Texture state after loadTexturesIntoGPU:");
	    OMLOGD(" - tex_ka.handle=%d", (int)tex_ka.handle);
	    OMLOGD(" - tex_kd.handle=%d", (int)tex_kd.handle);
	    OMLOGD(" - tex_ks.handle=%d", (int)tex_ks.handle);
	    OMLOGD(" - tex_bump.handle=%d", (int)tex_bump.handle);
	}

	/** Unload all textures from the main memory */
	void TextureDataHoldingMaterial::unloadTexturesFromMemory() {
	    // Clear texture data in memory
	    tex_ka.unloadBitmapFromMemory();
	    tex_kd.unloadBitmapFromMemory();
	    tex_ks.unloadBitmapFromMemory();
	    tex_bump.unloadBitmapFromMemory();
	    memoryHoldingState = TextureLoadState::NOT_LOADED;
	}

	/** Unload all textures from the GPU-memory. Unload is only called on non-zero handles */
	void TextureDataHoldingMaterial::unloadTexturesFromGPU(const GpuTexturePreparationLibrary &textureLib) {
	    // Only call the unload on those handles that are non-zero
	    if(tex_ka.handle != 0) { textureLib.unloadFromGPU(tex_ka); tex_ka.handle = 0; }
	    if(tex_kd.handle != 0) { textureLib.unloadFromGPU(tex_kd); tex_kd.handle = 0; }
	    if(tex_ks.handle != 0) { textureLib.unloadFromGPU(tex_ks); tex_ks.handle = 0; }
	    if(tex_bump.handle != 0) { textureLib.unloadFromGPU(tex_bump); tex_bump.handle = 0; }
	    gpuHoldingState = TextureLoadState::NOT_LOADED;
	}

}
