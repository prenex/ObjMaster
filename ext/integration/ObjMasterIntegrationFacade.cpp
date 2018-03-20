#include "ObjMasterIntegrationFacade.h"
#include "../../Obj.h"
#include "../../ObjCreator.h"
#include "../../MaterializedObjModel.h"
#include "../../NopTexturePreparationLibrary.h"
#include "../../FileAssetLibrary.h"
#include "../../TextureDataHoldingMaterial.h"
#include <algorithm>

/** This is a mapping of all the already loaded models - caching them in case of reload. The key is (path+filename) */
static std::unordered_map<std::string, int> modelMap;

// We do not have any texture preparation library as the unity side is the one that should handle that somehow
/** The vector of loaded models, handles are indices in this vector! */
static std::vector<ObjMaster::MaterializedObjModel<ObjMaster::NopTexturePreparationLibrary>> models;

/** The vector of ObjCreators - for saving and generating *.obj files */
static std::vector<ObjMaster::ObjCreator> creators;

/** This block contains the public interface of the dynamic library */
extern "C" {
#pragma region PUBLIC_DLL_API

	/** Just a test-function for checking DLL linkage success */
	void testSort(int a[], int length) {
		std::sort(a, a+length);
	}

	// Obj loading and handling functions
	// ==================================

	/** 
	  * Load the given obj with the objmaster system and return the handle for referencing it.
	  * The system uses caching so asking for the same, already loaded model ends up returning the same model reference.
	  * If the model already exists and not loaded, we reload it into memory from scratch and the old state will be destroyed!
	  *
	  * Returns -1 on errors, otherwise the valid handle for the model that has been loaded
	  */
	// The handle is the index in the loadedModels vector
	int loadObjModel(const char* path, const char* fileName) {
		return loadObjModelExt(path, fileName, true);	// reload earlier when necessary
	}

	/**
	 * Try to unload only the specific model.
	 * The model cache will contain a placeholder object afterwards which is not using big resources.
	 * Returns true if handle was valid and referenced a model which is in the unloaded state after the method.
	 */
	bool unloadObjModel(int handle) {
		try {
			// Becuase we don't remove the placeholders on unload
			// this check ensures we have something to act upon
			if ((int)models.size() > handle) {
				// If the model is inited, replace it with an empty model (this should free most resources while keeping other handles intact)
				if (models[handle].inited) {
					models[handle] = ObjMaster::MaterializedObjModel<ObjMaster::NopTexturePreparationLibrary>();
				}
				// If we are here, the model is already unloaded - either by us or someone else earlier!
				return true;
			}
			else {
				// Nothing to act upon - erronous call so return false!
				return false;
			}
		}
		catch (...) {
			return false; // hopefully never happens
		}
	}

	/**
	 * Tries to unload everything and release all resources. Returns true in case of success and false if something went wrong!
	 * After this operation, every handle is invalidated and gets unusable!
	 */
	bool unloadEverything() {
		try {
			// Release models: Proper RAII should solve everything here. If not, then there is some weird error in objmaster which is of course possible.
			models = std::vector<ObjMaster::MaterializedObjModel<ObjMaster::NopTexturePreparationLibrary>>();
			// Of course the cache should get to be empty once again now too!
			modelMap = std::unordered_map<std::string, int>();
			// Close all factories - as they are also resources
			closeAllFactories(); // In the terminology here, we call them factories...

			// Indicate success
			return true;
		}
		catch (...) {
			return false; // hopefully never happens as this might mean nasty stuff
		}
	}

	/**
	 * Returns the number of meshes in the model. Useful for later queries for iterating over all.
	 *
	 * Because we seperate new meshes for different materials,
	 * this value can be greater than what you see in a content creation tool like blender!
	 * Returns: -1 in case of errors (like a bad handle) and 0 if there is no mesh in the model!
	 *          The latter can happen also in the cases of not loaded/unloaded meshes!
	 */
	int getModelMeshNo(int handle) {
		if ((int)models.size() > handle) {
			// Return the number of meshes
			return (int)models[handle].meshes.size();
		}
		else {
			// Invalid handle!
			return -1;
		}
	}

	/**
	 * Returns the "objMatFaceGroup" name of the mesh. This is basically the 'g' or 'o' name and the material name together in this form: <objGroupName>:mtl:<matName>.
	 * To aid optimal rendering and grouping, geometry data is provided in chunks in which they belong to the same object/group and use the same material. Basically this function query the grouping key.
	 * The returned pointer is bound to the std::string in the C++ side of the loaded mesh, so users better make an instant copy!
	 */
	const char* getModelMeshObjMatFaceGroupName(int handle, int meshIndex) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// Return the pointer to the underlying c_str. This is okay as the user will immediately copy it as they are told to...
				return models[handle].meshes[meshIndex].name.c_str();
			}
			else {
				// Invalid handle or mesh index! Return a nullptr!
				return nullptr;
			}
		}
		catch (...) {
				// Something went wrong! Return nullptr!
				return nullptr;
		}
	}

	/**
	 * Returns the name of the material for the given mesh. The returned pointer is bound to the std::string in the C++ side of the loaded material in the mesh, so users better make an instant copy!
	 */
	const char* getModelMeshMaterialName(int handle, int meshIndex) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// Return the pointer to the underlying c_str. This is okay as the user will immediately copy it as they are told to...
				return models[handle].meshes[meshIndex].material.name.c_str();
			}
			else {
				// Invalid handle or mesh index! Return a nullptr!
				return nullptr;
			}
		}
		catch (...) {
				// Something went wrong! Return nullptr!
				return nullptr;
		}
	}
	
	/**
	 * Returns the simplified material descriptor for the mesh deignated by meshindex in the model designated by the handle.
	 * Might return empty material in case of errors (both bad handle and index errors or other problems and exceptions)
	 */
	SimpleMaterial getModelMeshMaterial(int handle, int meshIndex) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// Prepare the simple material to return
				SimpleMaterial sm;
				// enabled fields (useful for further queries too) TODO: what if we have more fields than the uint??
				sm.enabledFields = (unsigned int) models[handle].meshes[meshIndex].material.enabledFields.to_ulong();

				// Direct color fields
				// ka
				if (models[handle].meshes[meshIndex].material.ka.size() > 0) {
					sm.kar = models[handle].meshes[meshIndex].material.ka[0];
				}
				if (models[handle].meshes[meshIndex].material.ka.size() > 1) {
					sm.kag = models[handle].meshes[meshIndex].material.ka[1];
				}
				if (models[handle].meshes[meshIndex].material.ka.size() > 2) {
					sm.kab = models[handle].meshes[meshIndex].material.ka[2];
				}
				if (models[handle].meshes[meshIndex].material.ka.size() > 3) {
					sm.kaa = models[handle].meshes[meshIndex].material.ka[3];
				}
				else {
					sm.kaa = 1.0f;
				}
				// kd
				if (models[handle].meshes[meshIndex].material.kd.size() > 0) {
					sm.kdr = models[handle].meshes[meshIndex].material.kd[0];
				}
				if (models[handle].meshes[meshIndex].material.kd.size() > 1) {
					sm.kdg = models[handle].meshes[meshIndex].material.kd[1];
				}
				if (models[handle].meshes[meshIndex].material.kd.size() > 2) {
					sm.kdb = models[handle].meshes[meshIndex].material.kd[2];
				}
				if (models[handle].meshes[meshIndex].material.kd.size() > 3) {
					sm.kda = models[handle].meshes[meshIndex].material.kd[3];
				}
				else {
					sm.kda = 1.0f;
				}
				// ks
				if (models[handle].meshes[meshIndex].material.ks.size() > 0) {
					sm.ksr = models[handle].meshes[meshIndex].material.ks[0];
				}
				if (models[handle].meshes[meshIndex].material.ks.size() > 1) {
					sm.ksg = models[handle].meshes[meshIndex].material.ks[1];
				}
				if (models[handle].meshes[meshIndex].material.ks.size() > 2) {
					sm.ksb = models[handle].meshes[meshIndex].material.ks[2];
				}
				if (models[handle].meshes[meshIndex].material.ks.size() > 3) {
					sm.ksa = models[handle].meshes[meshIndex].material.ks[3];
				}
				else {
					sm.ksa = 1.0f;
				}

				// Return the created SimpleMaterial
				return sm;
			}
			else {
				// Invalid handle or mesh index! Return a material with no fields at all!
				return SimpleMaterial{
					0
				};
			}
		}
		catch (...) {
				// Something went wrong! Return a material with no fields at all!
				return SimpleMaterial{
					0
				};
		}
	}

	/** Tells the number of vertex data for the given mesh of the handle. Returns -1 in case of errors and zero when there is no data at all! */
	int getModelMeshVertexDataCount(int handle, int meshIndex) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// If we are here, we have valid handle and mesh index
				return models[handle].meshes[meshIndex].vertexCount;
			}
			else {
				return -1; // -1 indicates error
			}
		}
		catch (...) {
			return -1;	// Exceptions will not pass through the boundaries of the library!
		}
	}

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
	int getModelMeshBaseVertexOffset(int handle, int meshIndex) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// If we are here, we have valid handle and mesh index
				return models[handle].meshes[meshIndex].baseVertexLocation;
			}
			else {
				return -1; // -1 indicates error
			}
		}
		catch (...) {
			return -1;	// Exceptions will not pass through the boundaries of the library!
		}
	}

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
	int getModelMeshVertexData(int handle, int meshIndex, VertexStructure** output) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// Rem.: A models mesh can share their vector with the other meshes for optimization
				//       because of this, we need to return the vertex data only from the base location!
				int vertexCount = models[handle].meshes[meshIndex].vertexCount;
				int vertexBase = models[handle].meshes[meshIndex].baseVertexLocation;
				//// Copy the relevant part of the vector for the user
				//for (int i = vertexBase; i < vertexBase + vertexCount; ++i) {
				//	VertexStructure vs = (*(models[handle].meshes[meshIndex].vertexData))[i];
				//	output[i - vertexBase] = vs;
				//}
				// Marshalling takes place at the consumer - this way we have direct access
				// for non-managed languages at least! See C++11 reference about why we can
				// use the 
				VertexStructure* dataPtr = &((*(models[handle].meshes[meshIndex].vertexData))[vertexBase]);
				// The consumer side 
				*output = dataPtr;

				return vertexCount;	// Indicate success with the number of vertices
			}
			else {
				return -1;	// Indicate error because of invalid handle or index
			}
		}
		catch (...) {
			return -1;	// Exceptions will not pass through the boundaries of the library!
		}
	}

	/** Tells the number of index data for the given mesh of the handle. Returns -1 in case of errors and zero when there is no data at all! */
	int getModelMeshIndicesCount(int handle, int meshIndex) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// If we are here, we have valid handle and mesh index
				return models[handle].meshes[meshIndex].indexCount;
			}
			else {
				return -1; // -1 indicates error
			}
		}
		catch (...) {
			return -1;	// Exceptions will not pass through the boundaries of the library!
		}
	}

	/**
	 * Extracts the index-data for the mesh of the given handle into output.
	 * 
	 * The output should be a pointer to the pointer for the indices array we will return.
	 * 
	 * Returns -1 in case of errors or incomplete operation, otherwise we return the number of found indices!
	 *
	 * See: getModelMeshIndicesCount
	 */
	int getModelMeshIndices(int handle, int meshIndex, unsigned int** output) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// Rem.: A models mesh can share their vector with the other meshes for optimization
				//       because of this, we need to return the copy of the vertex data only from the base location!
				int iCount = models[handle].meshes[meshIndex].indexCount;
				int iBase = models[handle].meshes[meshIndex].startIndexLocation;
				// TODO: ensure that bit-width will work in all architectures we need it to work

				//// Copy the relevant part of the vector for the user
				//for (int i = iBase; i < iBase + iCount; ++i) {
				//	uint32_t index = (*(models[handle].meshes[meshIndex].indices))[i];
				//	output[i - iBase] = (unsigned int) index;
				//}

				// Give a reference 
				*output = &(*(models[handle].meshes[meshIndex].indices))[iBase];

				return iCount;	// Indicate success
			}
			else {
				return -1;	// error because of invalid handle or index
			}
		}
		catch (...) {
			return -1;	// Exceptions will not pass through the boundaries of the library!
		}
	}


	// Rem.: The handle is the index in the loadedModels vector
	/** 
	 * Load the given obj with the objmaster system and return the handle for referencing it.
	 * The system uses caching so asking for the same, already loaded model ends up returning the same model reference.
	 * If the model already exists and not loaded, we reload it into memory from scratch and the old state will be destroyed!
	 *
	 * Returns -1 on errors, otherwise the valid handle for the model that has been loaded
	 */
	int loadObjModelExt(const char* path, const char* fileName, bool reloadEarlier) {
		try {
			// Calculate the key for the model map cache
			std::string modelMapKey = std::string(path);
			modelMapKey += fileName;

			// Try finding the model in the cache
			bool needToReloadEarlier = false;
			int earlierLoadIndex = -1; // -1 as unused until (needToReloadEarlier == true)
			if (modelMap.find(modelMapKey) != modelMap.end()) {
				earlierLoadIndex = modelMap[modelMapKey];
				if (models[earlierLoadIndex].inited) {
					// We have found it and it is loaded
					return earlierLoadIndex;
				}
				else {
					// We have found it, but it is not loaded
					// -> reload would be necessary!
					// According to the flag we might reload or fail
					if (reloadEarlier) {
						needToReloadEarlier = true;
					}
					else {
						// Fail: we could reload as new, but that would disintegrate our caching!
						return -1;
					}
				}
			}

			// ///////////////
			// IF WE ARE HERE:
			// - We have not found the model in the cache
			// - Or we have found it, but it need to be reloaded and we have the index

			// Load and parse the obj model with its representation
			// Memory should be freed when leaving the method because of RAAI - at least I hope so!
			ObjMaster::Obj obj = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), path, fileName);

			// Create a model out of this obj and put it into the vector. We use move to try not to copy stuff.
			// We do not have any texture preparation library as the unity side is the one that should handle that somehow
			if (needToReloadEarlier) {
				// Exchange the old array element with the newly loaded model
				models[earlierLoadIndex] = std::move(ObjMaster::MaterializedObjModel<ObjMaster::NopTexturePreparationLibrary>(obj));
				// Return earlier handle as it is at that position now too after reload!
				return earlierLoadIndex;
			}
			else {
				// Add the new model to the end of the vector
				models.push_back(std::move(ObjMaster::MaterializedObjModel<ObjMaster::NopTexturePreparationLibrary>(obj)));

				// Calculate the "handle" as the model index
				int modelIndex = (int)models.size() - 1;

				// Return handle
				return modelIndex;
			}
		}
		catch (...)
		{
			// -1 indicates errors
			return -1;
		}
	}

	/**
	 * Returns the pointer to the null terminated fileName or nullptr in case of errors. If there is no texture file for the one asked for, we return an empty string!
	 */
	const char* getModelMeshAmbientTextureFileName(int handle, int meshIndex) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// We have valid handle and mesh
				return models[handle].meshes[meshIndex].material.map_ka.c_str();
			}
			else {
				return nullptr;	// error because of invalid handle or index
			}
		}
		catch (...) {
			return nullptr;	// Exceptions will not pass through the boundaries of the library!
		}
	}

	/**
	 * Returns the pointer to the null terminated fileName or nullptr in case of errors. If there is no texture file for the one asked for, we return an empty string!
	 */
	const char* getModelMeshDiffuseTextureFileName(int handle, int meshIndex) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// We have valid handle and mesh
				return models[handle].meshes[meshIndex].material.map_kd.c_str();
			}
			else {
				return nullptr;	// error because of invalid handle or index
			}
		}
		catch (...) {
			return nullptr;	// Exceptions will not pass through the boundaries of the library!
		}
	}

	/**
	 * Returns the pointer to the null terminated fileName or nullptr in case of errors. If there is no texture file for the one asked for, we return an empty string!
	 */
	const char* getModelMeshSpecularTextureFileName(int handle, int meshIndex) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// We have valid handle and mesh
				return models[handle].meshes[meshIndex].material.map_ks.c_str();
			}
			else {
				return nullptr;	// error because of invalid handle or index
			}
		}
		catch (...) {
			return nullptr;	// Exceptions will not pass through the boundaries of the library!
		}
	}

	/**
	 * Returns the pointer to the null terminated fileName or nullptr in case of errors. If there is no texture file for the one asked for, we return an empty string!
	 */
	const char* getModelMeshNormalTextureFileName(int handle, int meshIndex) {
		try {
			if ((int)models.size() > handle && (int)models[handle].meshes.size() > meshIndex) {
				// We have valid handle and mesh
				return models[handle].meshes[meshIndex].material.map_bump.c_str();
			}
			else {
				return nullptr;	// error because of invalid handle or index
			}
		}
		catch (...) {
			return nullptr;	// Exceptions will not pass through the boundaries of the library!
		}
	}
	// Obj creation/generation functions
	// =================================

	// COMMONMACROS:
// Rem.: size_t casting is here to ensure that -1 counts as a big value and is not available!
// check if the handle exists at least - return false otherwise
#define CHECK_HANDLE if((size_t)factoryHandle > creators.size()) { return false; }
// check if the handle exists at least - return -1 otherwise
#define CHECK_HANDLE_RETNEG if((size_t)factoryHandle > creators.size()) { return -1; }

	// 1.) Creation / closure
	// ----------------------

	// Rem.: The returned handle is the index in the creators vector
	/**
	  * Creates an ObjCreator factory for runtime Obj generation and an empty *.obj model in it and return the handle for the factory.
	  * - returns a negative value in case of errors!
	  * Rem.: Useful when creating / saving models from scratch.
	  */
	int createObjFactory() {
		// Rem.: nullptr in the fileName means to create empty
		// Rem.: delegating also handles errors properly
		return createObjFactoryWithBaseObj(nullptr, nullptr);
	}

	// Rem.: The returned handle is the index in the creators vector
	/**
	  * Creates an ObjCreator factory for runtime Obj generation by loading the given *.obj model in it as a base and return the handle for the factory to extend this geometry.
	  * Rem.: The file designated by path+fileName is not affected or changed, unless there is a save operation as that designation as its target!
	  * Rem.: Useful when "appending" new data to an already existing obj file (output can be saved as a different obj however)
	  */
	int createObjFactoryWithBaseObj(const char* path, const char* fileName) {
		try{
			// The index will be the current size
			int ret = creators.size();
			if(fileName != nullptr) {
				// Create using the parsed *.obj as a basis to append data to
auto test = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), path, fileName);
				creators.push_back(std::move(ObjMaster::ObjCreator(std::move(ObjMaster::Obj(ObjMaster::FileAssetLibrary(), path, fileName)))));
			}else{
				// Create empty
				creators.push_back(ObjMaster::ObjCreator());
			}

			// Return the id of the creator (as a handle)
			return ret;
		} catch (...) {
			return -1;	// -1 is always a bad handle and is a good indicator is failure
		}
	}

	/**
	 * Can be used to CLOSE/reset THE FACTORY and save a *.obj file out created by the given factory handle to the given path - in the same time.
	 * - Return value of false indicates that there was some error in this operation!
	 * Rem.: Really useful in case we just created a factory "temporally" (like we would do in case of a local stack-object if we were in C++ / OOP)
	 *       and we want to have a method to release its resources immediately (for example generiting **lots** of *.obj files in bulk!)
	 */
	bool saveObjFromFactoryToFileAndPossiblyCloseFactory(int factoryHandle, const char* path, const char* fileName, bool closeFactory = true){
		// check if the handle exists at least...
		CHECK_HANDLE
		// Try to save the file
		try{
			ObjMaster::ObjCreator &creator = creators[factoryHandle];
			// Rem.: Here we can safely refer to the owned data without making a copy
			//       as we will just save the obj and would throw away the copy anyways!
			ObjMaster::Obj *created = creator.getOwnedObj();
			created->saveAs(ObjMaster::FileAssetLibrary(), path, fileName);

			if(closeFactory) {
				// Close and indicate also if closing was a success or not
				return hintCloseFactory(factoryHandle);
			}else{
				// Indicate success
				return true;
			}
		}catch(...){
			return false;
		}
	}

	/**
	 * Save a *.obj file out created by the given factory handle to the given path.
	 * - Return value of false indicates that there was some error in this operation!
	 * Rem.: This keeps the factory open so beware of the memory leaking! If you don't want to reuse the factory, better reset it or close all factories!
	 */
	bool saveObjFromFactoryToFile(int factoryHandle, const char* path, const char* fileName){
		return saveObjFromFactoryToFileAndPossiblyCloseFactory(factoryHandle, path, fileName, false);
	}

	/**
	 * Resets the given factory to a minimal resource usage and an empty underlying Obj - if there is no such handle, the operation is undefined!
	 * - Return value of false indicates that there was some error in this operation!
	 */
	bool resetFactory(int factoryHandle) {
		// check if the handle exists at least...
		CHECK_HANDLE
		try{
			// RAII is useful to "delete" the earlier factory if needed.
			creators[factoryHandle] = ObjMaster::ObjCreator();
			return true;
		}catch(...){
			return false;
		}
	}

	/**
	  * The system tries its best to release all resources aquired by the factory: it is not defined if leaks are possible of not - so if you do not want leaks then use closeAllFactories()!
	  * This saves a bit of memory and makes the factory handle unusable! In good implementations it should be safe to rely on this method saving most resources that is possible.
	 * - Return value of false indicates that there was some error in this operation!
	  */
	bool hintCloseFactory(int factoryHandle) {
		// check if the handle exists at least...
		CHECK_HANDLE
		try{
			// RAII is useful to "delete" the earlier factory if needed.
			// Rem.: using nullptr here uses the Obj* constructor in which we
			//       are saving the most memory by not creating any self-owned object!
			// Rem.: This really makes the handle 
			creators[factoryHandle] = ObjMaster::ObjCreator(nullptr);
			return true;
		}catch(...){
			return false;
		}
	}

	/**
	 * Closes all ObjCreator factories created by the ObjMasterIntegrationFacade. Invalidates all factory handles!!!
	 * - Return value of false indicates that there was some error in this operation!
	 */
	bool closeAllFactories() {
		try{
			// Seting the creators to an empty vector means releasing all resources with RAII.
			creators = std::vector<ObjMaster::ObjCreator>();
			return true;
		}catch(...){
			return false;
		}
	}

	// 2.) Runtime generation of *.obj geometry and materials
	// ------------------------------------------------------

	/**
	 * Adds the given runtime-generated material to the given factory - beware of filling "enabledFields" properly!
	 * Rem.: texture names must be added directly to easy p/invoke or JNI and such integrations. The texture is only used, if the m.enabledFields says so however!
	 * Rem.: Because the texture name goes unused for not enabled fields, in those cases they are better kept as nullptrs.
	 */
	bool addRuntimeGeneratedMaterial(int factoryHandle, SimpleMaterial m, const char *materialName,
		       const char *map_ka, const char *map_kd, const char *map_ks, const char *map_bump) {
		// check if the handle exists at least...
		CHECK_HANDLE
		// Create an ObjMaster-side Material object corresponding to the given data
		ObjMaster::Material mat;
		mat.enabledFields = m.enabledFields;
		mat.ka = std::vector<float>{m.kar, m.kag, m.kab, m.kaa};
		mat.kd = std::vector<float>{m.kdr, m.kdg, m.kdb, m.kda};
		mat.ks = std::vector<float>{m.ksr, m.ksg, m.ksb, m.ksa};
		mat.map_ka = map_ka;
		mat.map_kd = map_kd;
		mat.map_ks = map_ks;
		mat.map_bump = map_bump;
		// Add the material using the ObjCreator this factory refers to
		creators[factoryHandle].addRuntimeGeneratedMaterial(std::move(mat));

		// Indicate success
		return true;
	}

	/**
	  * Unsafe and fast: adds a vertex structure to the Obj
	  * - We expect that the Obj is build only using this method and nothing else.
	  * - The index of the added 'v' is returned - or in case of errors: (-1)
	  * - This index can be used as a global index for everything in the vData (like uvs and normals too)
	  * - However the latter only works if we do not use the objs support of "different indices per element" logic!
	  *   This means that we can easily bulk-generate output for data already in a renderer-friendly format, but
	  *   we better not mix generating data from that with "extending" already opened obj files for example!
	  */
	int addHomogenousVertexStructure(int factoryHandle, VertexStructure vData) {
		// check if the handle exists at least - return -1 otherwise
		CHECK_HANDLE_RETNEG
		// delegate the call towards the selected factory handle
		return creators[factoryHandle].unsafeAddVertexStructure(vData);
	}

	/**
	  * Add a face with homogenous indices (v, vt, vn indices are the same).
	  * Three index is necessary for forming a triangle!
	  *
	  * Returns the face-Index or (-1) in case of errors.
	  */
	int addFace(int factoryHandle, unsigned int aIndex, unsigned int bIndex, unsigned int cIndex) {
		// check if the handle exists at least - return -1 otherwise
		CHECK_HANDLE_RETNEG
		// delegate the call towards the selected factory handle
		return creators[factoryHandle].addFace(aIndex, bIndex, cIndex);
	}

	/** Use the given group-name from now on - every face will be in the group. */
	int useGroup(int factoryHandle, const char *name) {
		// check if the handle exists at least - return -1 otherwise
		CHECK_HANDLE_RETNEG
		// delegate the call towards the selected factory handle
		return creators[factoryHandle].useGroup(std::string(name));
	}

	/** Use the given material-name from now on - every face will have the given material. If materialName not exists, an empty material gets generated! */
	int useMaterial(int factoryHandle, const char *materialName) {
		// check if the handle exists at least - return -1 otherwise
		CHECK_HANDLE_RETNEG
		// delegate the call towards the selected factory handle
		return creators[factoryHandle].useMaterial(std::string(materialName));
	}

#pragma endregion
}
