/**
 * Integration facade DLL/SO for the ObjMaster library.
 *
 * Useful for managed language integration or C-language binding.
 * Originally written for providing C#/.NET mono integration to the Unity game engine.
 *
 * Tested with: VS 2015 + unity hololens preview 5.4.0f3 versions but might work as a simple C-binding...
 */
#pragma once
#ifndef OBJ_MASTER_INTEGR_FACADE_H
#define OBJ_MASTER_INTEGR_FACADE_H

#include "../../VertexStructure.h"

#define DLL_API __declspec(dllexport) 
// Comment this out, if you are compiling statically into your project (with gcc for example)
//#define DLL_API static

extern "C" {
	/** Just a test-function for checking DLL linkage success */
	DLL_API void testSort(int a[], int length);

	/** A simplified material structure for integration */
	struct SimpleMaterial {
		unsigned int enabledFields; // USE Material.F_* values for indexing! Also shows if we can ask for texture names or not!
		float kar, kag, kab, kaa;	// Only relevant if enabledFields shows it is!
		float kdr, kdg, kdb, kda;	// Only relevant if enabledFields shows it is!
		float ksr, ksg, ksb, ksa;	// Only relevant if enabledFields shows it is!
	};

	/** 
	  * Load the given obj with the objmaster system and return the handle for referencing it.
	  * The system uses caching so asking for the same, already loaded model ends up returning the same model reference.
	  * If the model already exists and not loaded, we reload it into memory from scratch and the old state will be destroyed!
	  *
	  * Returns -1 on errors, otherwise the valid handle for the model that has been loaded
	  */
	DLL_API int loadObjModel(const char* path, const char* fileName);

	/** 
	  * Load the given obj with the objmaster system and return the handle for referencing it.
	  * The system uses caching so asking for the same, already loaded model ends up returning the same model reference.
	  * If the model already exists and not loaded, we reload it into memory from scratch and the old state will be destroyed!
	  *
	  * Returns -1 on errors, otherwise the valid handle for the model that has been loaded
	  */
	DLL_API int loadObjModelExt(const char* path, const char* fileName, bool reloadEarlier);

	/**
	 * Try to unload only the specific model.
	 * The model cache will contain a placeholder object afterwards which is not using big resources.
	 * Returns true if handle was valid and referenced a model which is in the unloaded state after the method.
	 */
	DLL_API bool unloadObjModel(int handle);

	/** Tries to unload everything and release all resources. Returns true in case of success and false if something went wrong! */
	DLL_API bool unloadEverything();

	/**
	  * Returns the number of meshes in the model. Useful for later queries.
	  *
	  * Because we seperate new meshes for different materials,
	  * this value can be greater than what you see in a content creation tool like blender!
	  * Returns: -1 in case of errors (like a bad handle) and 0 if there is no mesh in the model!
	  *          The latter can happen also in the cases of not loaded/unloaded meshes!
	  */
	DLL_API int getModelMeshNo(int handle);

	/**
	 * Returns the simplified material descriptor for the mesh deignated by meshindex in the model designated by the handle.
	 * Might return empty material in case of errors (both bad handle and index errors or other problems and exceptions)
	 */
	DLL_API SimpleMaterial getModelMeshMaterial(int handle, int meshIndex);

	DLL_API int getModelMeshVertexDataCount(int handle, int meshIndex);

	DLL_API bool getModelMeshVertexDataCopy(int handle, int meshIndex, VertexStructure* output);

	/** Tells the number of vertex data for the given mesh of the handle. Returns -1 in case of errors and zero when there is no data at all! */
	DLL_API int getModelMeshIndicesCount(int handle, int meshIndex);

	/**
	 * Extracts the index-data for the mesh of the given handle into output.
	 * 
	 * The output should be big-enough to hold the index-data, so the user code
	 * should first ask for the number of the indices and allocate buffer!
	 * 
	 * Returns false in case of errors or incomplete operation
	 *
	 * See: getModelMeshIndicesCount
	 */
	DLL_API bool getModelMeshIndicesCopy(int handle, int meshIndex, unsigned int* output);
}

#endif
