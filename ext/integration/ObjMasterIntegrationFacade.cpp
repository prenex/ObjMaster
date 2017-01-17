#include "ObjMasterIntegrationFacade.h"
#include "../../Obj.h"
#include "../../MaterializedObjModel.h"
#include "../../NopTexturePreparationLibrary.h"
#include "../../FileAssetLibrary.h"
#include "../../TextureDataHoldingMaterial.h"
#include <algorithm>

/** This is a mapping of all the already loaded models - caching them in case of reload. The key is (path+filename) */
std::unordered_map<std::string, int> modelMap;

// We do not have any texture preparation library as the unity side is the one that should handle that somehow
/** The vector of loaded models, handles are indices in this vector! */
std::vector<ObjMaster::MaterializedObjModel<ObjMaster::NopTexturePreparationLibrary>> models;

/** This block contains the public interface of the dynamic library */
extern "C" {
#pragma region PUBLIC_DLL_API

	/** Just a test-function for checking DLL linkage success */
	void testSort(int a[], int length) {
		std::sort(a, a+length);
	}

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
			if (models.size() > handle) {
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

	/** Tries to unload everything and release all resources. Returns true in case of success and false if something went wrong! */
	bool unloadEverything() {
		try {
			// Proper RAII should solve everything here. If not, then there is some weird error in objmaster which is of course possible.
			models = std::vector<ObjMaster::MaterializedObjModel<ObjMaster::NopTexturePreparationLibrary>>();
			// Of course the cache should get to be empty once again now too!
			modelMap = std::unordered_map<std::string, int>();
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
		if (models.size() > handle) {
			// Return the number of meshes
			return models[handle].meshes.size();
		}
		else {
			// Invalid handle!
			return -1;
		}
	}

	/**
	 * Returns the simplified material descriptor for the mesh deignated by meshindex in the model designated by the handle.
	 * Might return empty material in case of errors (both bad handle and index errors or other problems and exceptions)
	 */
	SimpleMaterial getModelMeshMaterial(int handle, int meshIndex) {
		try {
			if (models.size() > handle && models[handle].meshes.size() > meshIndex) {
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
			if (models.size() > handle && models[handle].meshes.size() > meshIndex) {
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
			if (models.size() > handle && models[handle].meshes.size() > meshIndex) {
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
			if (models.size() > handle && models[handle].meshes.size() > meshIndex) {
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
			if (models.size() > handle && models[handle].meshes.size() > meshIndex) {
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


	/** 
	  * Load the given obj with the objmaster system and return the handle for referencing it.
	  * The system uses caching so asking for the same, already loaded model ends up returning the same model reference.
	  * If the model already exists and not loaded, we reload it into memory from scratch and the old state will be destroyed!
	  *
	  * Returns -1 on errors, otherwise the valid handle for the model that has been loaded
	  */
	// The handle is the index in the loadedModels vector
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
				int modelIndex = models.size() - 1;

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
#pragma endregion
}
