/**
 * Integration facade layer/dll/so for the ObjMaster library.
 *
 * Useful for managed language integration or C-language binding.
 * Originally written for providing C#/.NET mono integration to the Unity game engine.
 *
 * Tested with: VS 2015 + unity hololens preview 5.4.0f3 versions but can work as a simple C-binding.
 * For building, just create an empty DLL project and add objmaster sources with this.
 */
#pragma once
#ifndef OBJ_MASTER_INTEGR_FACADE_H
#define OBJ_MASTER_INTEGR_FACADE_H

// When the facade should not be built as DLL, you need to define this!
#define NO_OBJMASTER_FACADE_DLL 1	// needed to get rid of the DLL specific stuff

#include "../../VertexStructure.h"

#ifndef NO_OBJMASTER_FACADE_DLL
	#define DLL_API __declspec(dllexport) 
#else
	#define DLL_API extern
#endif // NO_OBJMASTER_FACADE_DLL

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

	/** Tells the number of vertex data for the given mesh of the handle. Returns -1 in case of errors and zero when there is no data at all! */
	DLL_API int getModelMeshVertexDataCount(int handle, int meshIndex);

	/**
	 * Returns the base-offset of vertices in this mesh when using shared vertex buffers.
	 * When the vertex buffers are given per-mesh, this always return 0 for safe usage.
	 * This method is really useful for creating per-mesh buffers from the objmaster provided
	 * data as it is in the case of the unity integration and such. Indices of one mesh will
	 * have an offset of this value in the shared case so creating a copy of the real indices
	 * works by getting this value and substracting it from each shared index value!
	 *
	 * Rem.: This method indicates errors by returning negative values (-1)
	 */
	DLL_API int getModelMeshBaseVertexOffset(int handle, int meshIndex);

	/**
	 * Extracts the vertex data for the mesh of the given handle into output.
	 * 
	 * The output is a pointer to the pointer that will get filled by the location of the data!
	 * Do not try to free this memory! It is handled by inner workings of objmaster! If the model
	 * or its mesh keeps unchanged, the memory areas will be valid - otherwise you should copy them!
	 * 
	 * Returns -1 in case of errors or incomplete operation, otherwise return the number of output vertices
	 *
	 * See: getModelMeshVertexDataCount if you only need this latter value
	 */
	DLL_API int getModelMeshVertexData(int handle, int meshIndex, VertexStructure** output);

	/** Tells the number of index data for the given mesh of the handle. Returns -1 in case of errors and zero when there is no data at all! */
	DLL_API int getModelMeshIndicesCount(int handle, int meshIndex);

	/**
	 * Extracts the index-data for the mesh of the given handle into output.
	 * 
	 * The output should be a pointer to the pointer for the indices array we will return.
	 * 
	 * Returns -1 in case of errors or incomplete operation, otherwise we return the number of found indices!
	 *
	 * See: getModelMeshIndicesCount
	 */
	DLL_API int getModelMeshIndices(int handle, int meshIndex, unsigned int** output);

	/**
	 * Returns the pointer to the null terminated fileName or nullptr in case of errors. If there is no texture file for the one asked for, we return an empty string!
	 */
	DLL_API const char* getModelMeshAmbientTextureFileName(int handle, int meshIndex);
	/**
	 * Returns the pointer to the null terminated fileName or nullptr in case of errors. If there is no texture file for the one asked for, we return an empty string!
	 */
	DLL_API const char* getModelMeshDiffuseTextureFileName(int handle, int meshIndex);
	/**
	 * Returns the pointer to the null terminated fileName or nullptr in case of errors. If there is no texture file for the one asked for, we return an empty string!
	 */
	DLL_API const char* getModelMeshSpecularTextureFileName(int handle, int meshIndex);
	/**
	 * Returns the pointer to the null terminated fileName or nullptr in case of errors. If there is no texture file for the one asked for, we return an empty string!
	 */
	DLL_API const char* getModelMeshNormalTextureFileName(int handle, int meshIndex);
}

#endif
