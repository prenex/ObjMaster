//
// Created by rthier on 2016.06.20..
//

#ifndef MATERIALIZEDOBJMODEL_H
#define MATERIALIZEDOBJMODEL_H

#include <vector>
#include <memory>
#include "MaterializedObjMeshObject.h"
#include "GpuTexturePreparationLibrary.h"
#include "Obj.h"

namespace ObjMaster {

    /**
     * Renderable 3D obj model with possible materials.
     *
     * A GpuTexturePreparationLibrary implementation should be provided as a template
     * parameter so that we can unload/load textures properly - even in the destructor.
     */
    template <class GpuTexturePreparationLibraryImpl>
    class MaterializedObjModel {
    public:
		bool inited = false;
		std::vector<MaterializedObjMeshObject> meshes;
		const char* path;

	/** Create a materialized obj model using the given obj representation */
	MaterializedObjModel(const Obj &obj) {
		// The path of the model is the same as the path for obj
		path = obj.objPath;
		// We create one mesh per each object material group
		for(auto gPair : obj.objectMaterialGroups) {
#ifdef DEBUG
        	OMLOGI("!!!!! Object material group have found as %s", gPair.first.c_str());
#endif
			meshes.push_back(std::move(MaterializedObjMeshObject(obj,
				&(obj.fs[gPair.second.faceIndex]),
				gPair.second.meshFaceCount,
				gPair.second.textureDataHoldingMaterial,
				gPair.first)));
		}

		// Indicate that the model is loaded
		inited = true;
	}

	/** Create a materialized obj model that is not inited (empty) */
	MaterializedObjModel() {}
	/** Destructor of the model - tries to unload all material groups textures */
	~MaterializedObjModel()	{ unloadAllTextures(); }

	/**
	 * Load the textures of meshes onto the GPU for rendering.
	 *
	 * This iterates through all meshes materials and do the following to it:
	 * - Load mesh material textures into the main memory (if applicable)
	 * - Load mesh material textures from main memory onto the GPU (if applicable)
	 * - Unload mesh material textures from the main memory
	 *
	 * The result of this operation is that all the materials texture binding 
	 * fields will be filled according to this and the model can be rendered!
	 */
	void loadAllTextures(const TexturePreparationLibrary &texLibrary) {
		for(auto &mesh : meshes) {
			mesh.material.loadTexturesIntoMemory(path, texLibrary);
			mesh.material.loadTexturesIntoGPU(gpuTexLibrary);
			mesh.material.unloadTexturesFromMemory();
		}
	}

	/** First unload all model textures from the GPU then also unload any textures from main memory */
	void unloadAllTextures() {
		for(auto &mesh : meshes) {
			mesh.material.unloadTexturesFromGPU(gpuTexLibrary);
			mesh.material.unloadTexturesFromMemory();
		}
	}
    private:
	/**
	 * Have to keep a library because the GPU-unload need to be managed by this! The unload
	 * from memory does not need the library, but unloading from the GPU is different and
	 * still we want to do that when the destructor is called so we are not leaking memory
	 * of the GPU. This way, when the model go out of scope or things like that, the cleanup
	 * works automatically. The normal texture prep library however is not cached as it can
	 * be convenient to load textures from varoius places - however it is not convenient to
	 * change the graphics API on the fly while using the same models so it is okay that the
	 * user code should supply this when constructing the model as template param.
	 */
		GpuTexturePreparationLibraryImpl gpuTexLibrary = GpuTexturePreparationLibraryImpl();
    };
}


#endif // MATERIALIZEDOBJMODEL_H
