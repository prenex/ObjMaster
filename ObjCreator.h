/**
  * Created by Thier Richard
  * u9vata@gmail.com
  * - 2018.03.12
  */
#ifndef _OBJMASTER_OBJ_CREATOR_H
#define _OBJMASTER_OBJ_CREATOR_H

#include<vector>
#include<cassert>
#include<algorithm> // for std::swap
#include"Obj.h"
#include"MtlLib.h"
#include"Material.h"
#include"VertexElement.h"
#include"VertexNormalElement.h"
#include"VertexTextureElement.h"
#include "VertexStructure.h"

namespace ObjMaster {
	/**
	* Useful for creating Obj objects in a structured way to save them out as  *.obj files.
	* The resulting representation is feasible for direct *.obj output using Obj::saveAs(..)
	* See Obj and similar low-level classes for the result of this factory / creator class.
	*
	* Basically this is just a convenience class so that creation of an Obj works easier than
	* just writing in the data directly into the lower level classes! Also the class shows off
	* nearly all healthy possibilities for generating *.obj files well!
	*/
	class ObjCreator final {
	public:

		// Constructor / Destructor
		// ------------------------
		// Copy construction
		void copyHelper(const ObjCreator &other) {
			// Mutable state for groups and materials - just copy these
			currentObjMatFaceGroupStart = other.currentObjMatFaceGroupStart;
			currentGrpName = other.currentGrpName;
			currentMatName = other.currentMatName;
			createdWithBase = other.createdWithBase;
			// Update the obj pointer
			if(other.ownsObj) {
				// Copy-construct our obj - based on the other one
				obj = new Obj(*other.obj);
				ownsObj = true;
			} else {
				// We can copy the pointer and tell we do not own it
				// - just like our source that we copy
				obj = other.obj;
				ownsObj = false;
			}
		}
		ObjCreator(const ObjCreator &other) {
			// Do what we used to do when copy assigning
			copyHelper(other);
		}
		ObjCreator& operator=(const ObjCreator &other) {
			// This is NECESSARY here to avoid leaks!
			if(ownsObj) {
				delete obj;
			}

			copyHelper(other);
			return *this;
		}
		// MovMove construction
		void moveHelper(ObjCreator &&other){
			// Mutable state for groups and materials - just copy these and swap these
			createdWithBase = other.createdWithBase;
			currentObjMatFaceGroupStart = other.currentObjMatFaceGroupStart;
			std::swap(currentGrpName, other.currentGrpName);
			std::swap(currentMatName, other.currentMatName);
			obj = other.obj; // just get pointer
			ownsObj = other.ownsObj;   // Copy ownership flag from other!
			// Ensure that the other is not trying to release any resources if not need to!
			// Rem.: Necessary as other might get destructed right in the moment and must do it well!
			other.ownsObj = false;
		}
		ObjCreator(ObjCreator &&other) {

			// Do what we used to when move assigning
			moveHelper(std::move(other));
		}
		ObjCreator& operator=(ObjCreator &&other) {
			// This is NECESSARY here to avoid leaks!
			if(ownsObj) {
				delete obj;
			}

			moveHelper(std::move(other));
			return *this;
		}

		/** Create an empty factory. The underlying Obj is owned by the creator / factory */
		ObjCreator() {
			ownsObj = true;
			createdWithBase = false;
			obj = new Obj();
			// Set start index for current group face index
			currentObjMatFaceGroupStart = 0; //always zero: (int)(obj->fs.size());
		}

		/** Use the obj-creator to "extend" and already existing Obj. Be careful with this shared way of working! Obj is owned by the caller! */
		ObjCreator(Obj *notOwnedObj) {
			ownsObj = false;
			createdWithBase = true;
			obj = notOwnedObj;
			// Set start index for current group face index
			// Rem.: Really is needed as we need to "append" in this case!
			if(obj != nullptr) {
				currentObjMatFaceGroupStart = (int)(obj->fs.size());
			}
		}

		/** Use the obj-creator to "extend" and already existing Obj. The provided Obj will be cloned and the original unaffected! */
		ObjCreator(Obj &&objToCopy) {
			ownsObj = true;
			createdWithBase = true;
			obj = new Obj();
			*obj = std::move(objToCopy);
			// Set start index for current group face index
			// Rem.: Really is needed as we need to "append" in this case!
			currentObjMatFaceGroupStart = (int)(obj->fs.size());
		}

		/** Delete the creator and release all owned resources - basically we delete the underlying Obj if we own that and is not shared */
		~ObjCreator() {
			if(ownsObj) {
				delete obj;
			}
		}

		/** Gets the Obj and finish any possibly opened groups - but beware this might be owned by the creator! */
		inline Obj* getOwnedObj() {
			finishCurrentGroup();
			return obj;
		}

		/** Gets a copy of the Obj that is created so far - also finishes any possibly opened groups  */
		inline Obj getObj() {
			finishCurrentGroup();
			return *obj;
		}

		// Material library handling
		// -------------------------

		/** Set the whole MtlLib to the given one */
		inline void overWriteMtlLib(MtlLib newMtlLib) {
			obj->mtlLib = newMtlLib;
		}

		/** Add new material to the material lib */
		inline void addRuntimeGeneratedMaterial(Material m) {
			obj->mtlLib.addRuntimeGeneratedMaterial(std::forward<Material>(m));
		}

		/** Add new material to the material lib */
		inline void addMovedRuntimeGeneratedMaterial(Material &&m) {
			obj->mtlLib.addRuntimeGeneratedMaterial(std::forward<Material>(m));
		}
		
		// Adding geometry
		// ---------------

		// Methods for individual geometry additions - for using these one needs to understand the *.obj structure well

		/** LOW-LEVEL geometry manipulation: Adds a 'v' and returns the index to it */
		inline int llAddVertex(float x, float y, float z) {
			int index = obj->vs.size();
			obj->vs.emplace_back(x, y, z);
			return index;
		}

		/** LOW-LEVEL geometry manipulation: Adds a 'vt' and returns the index to it */
		inline int llAddVertexTexcoord(float u, float v) {
			int index = obj->vts.size();
			obj->vts.emplace_back(u, v);
			return index;
		}

		/** LOW-LEVEL geometry manipulation: Adds a 'vn' and returns the index to it */
		inline int llAddVertexNormal(float x, float y, float z) {
			int index = obj->vns.size();
			obj->vns.emplace_back(x, y, z);
			return index;
		}

		// Methods for adding as vertex structure

		/**
		  * Unsafe and fast: adds a vertex structure to the Obj
		  * - We expect that the Obj is build only using this method and nothing else.
		  * - The index of the added 'v' is returned!
		  * - This index can be used as a global index for everything in the vData (like uvs and normals too)
		  * - However the latter only works if we do not use the objs support of "different indices per element" logic!
		  *   This means that we can easily bulk-generate output for data already in a renderer-friendly format, but
		  *   we better not mix generating data from that with "extending" already opened obj files for example!
		  */
		inline int unsafeAddVertexStructure(VertexStructure vData) {
			// Do assertions in debug mode - even if we are "unsafe" and "fast" we should be debuggable!
#ifdef DEBUG
			int vindex = obj->vs.size();
			int vtindex = obj->vts.size();
			int vnindex = obj->vns.size();
			assert((vindex == vtindex) && (vtindex == vnindex));
#endif
			// Add vertex data directly the fast way
			// Rem.: because of inlining, most of the index getting code
			// should get erased by the compiler hopefully here!
			int ret = llAddVertex(vData.x, vData.y, vData.z);
			llAddVertexTexcoord(vData.u, vData.v);
			llAddVertexNormal(vData.i, vData.j, vData.k);

			// We return the index of the added 'v'
			return ret;
		}

		/**
		  * Useful in case of "extending" an already existing model with some new geometry!
		  * This method is always safe but might generate garbage / unused elements: adds a vertex structure to the Obj
		  * - The index of the added 'v', 'vt' and 'vn' is returned and is guaranteed to be the same for all the structure elements!
		  * - This index can be used as a global index for everything in the vData (like uvs and normals too)
		  *   The latter works by possibly added "padding-data" in cases the indices would differ. This might mean a lot of junk!!!
		  *   This means that we can easily bulk-generate output for data already in a renderer-friendly format, but
		  *   we better not mix generating data from that with "extending" already opened obj files for example as in those cases
		  *   a lot of junk might get generated even though this method is guaranteed to work.
		  */
		inline int safejunkAddVertexStructure(VertexStructure vData) {
			// Calculate how many "padding" we need for each data stride
			// Get the "would-be" indices of inserted data
			int vindex = obj->vs.size();
			int vtindex = obj->vts.size();
			int vnindex = obj->vns.size();
			// Rem.: This is a max-search on three elements here
			int maxIndex = (vtindex > vindex) ? vtindex : vindex;
			maxIndex = (vnindex > maxIndex) ? vnindex : maxIndex;
			// Calculate differences from the maximum to get paddings
			int vpad = maxIndex - vindex;
			int vtpad = maxIndex - vtindex;
			int vnpad = maxIndex - vnindex;

			// Add padding (just zeroes)
			// Rem.: Just imagine the mayhem this does
			//       when all the model shared the texture coordinates of
			//       four values! This can grow the mem and file size by 1/3!
			// 'v'
			for(int i = 0; i < vpad; ++i) {
				llAddVertex(0, 0, 0);
			}
			// 'vt'
			for(int i = 0; i < vtpad; ++i) {
				llAddVertexTexcoord(0, 0);
			}
			// 'vn'
			for(int i = 0; i < vnpad; ++i) {
				llAddVertexNormal(0, 0, 0);
			}
			OMLOGW("ObjCreator: %d padding vertices are added to reach common maxIndex = %d!", vpad, maxIndex);
			OMLOGW("ObjCreator: %d padding textcoords are added to reach common maxIndex = %d!", vtpad, maxIndex);
			OMLOGW("ObjCreator: %d padding normals are added to reach common maxIndex = %d!", vnpad, maxIndex);


			// Add vertex data directly - finally
			// Rem.: Because of above, now they share their indices
			// Rem.: because of inlining, most of the index getting code
			// should get erased by the compiler hopefully here!
			int ret = llAddVertex(vData.x, vData.y, vData.z);
			llAddVertexTexcoord(vData.u, vData.v);
			llAddVertexNormal(vData.i, vData.j, vData.k);

			// We return the index of the added 'v'
			// Rem.: This is surely the same as the 'vt' and 'vn' indices of the added data!
			return ret;
		}

		// Adding faces
		// ------------

		/**
		  * Add a face with non-homogenous indices (v, vt, vn indices can be different).
		  * 3x3 index is necessary for forming a triangle!
		  *
		  * Returns the face-Index
		  */
		inline int addFaceNonHomogenous(
		  unsigned int avIndex, 
		  unsigned int avtIndex, 
		  unsigned int avnIndex, 
		  unsigned int bvIndex, 
		  unsigned int bvtIndex, 
		  unsigned int bvnIndex, 
		  unsigned int cvIndex,
		  unsigned int cvtIndex,
		  unsigned int cvnIndex
		) {
			int ret = obj->fs.size();
			obj->fs.emplace_back(FaceElement{
					{avIndex, avtIndex, avnIndex},
					{bvIndex, bvtIndex, bvnIndex},
					{cvIndex, cvtIndex, cvnIndex},
			});
			return ret;
		}

		/**
		  * Add a face with homogenous indices (v, vt, vn indices are the same).
		  * Three index is necessary for forming a triangle!
		  *
		  * Returns the face-Index
		  */
		inline int addFace(unsigned int aIndex, unsigned int bIndex, unsigned int cIndex) {
			int ret = obj->fs.size();
			obj->fs.emplace_back(FaceElement{
					{aIndex, aIndex, aIndex},
					{bIndex, bIndex, bIndex},
					{cIndex, cIndex, cIndex},
			});
			return ret;
		}

		// Groups and useMtl handling
		// --------------------------

		/** Use the given group-name from now on - every face will be in the group. */
		inline int useGroup(std::string name) {
			// See if the group name differs from the new one or not - because otherwise it is NO-OP
			// Rem.: This way we get into the if clause when the first - unnamed - material or group
			//       gets closed too, which plays together very nicely with the Obj::saveAs workings!
			if(currentGrpName != name) {
				// Finish the earlier group first
				finishCurrentGroup();

				// Update "current" variables
				currentGrpName = name;
			}
			// Just return the ealier group start index
			return currentObjMatFaceGroupStart;
		}

		/** Use the given material-name from now on - every face will have the given material. If materialName not exists, an empty material gets generated! */
		inline int useMaterial(std::string materialName) {
			// See if the material name differs from the new one or not - because otherwise it is NO-OP
			// Rem.: This way we get into the if clause when the first - unnamed - material or group
			//       gets closed too, which plays together very nicely with the Obj::saveAs workings!
			if(currentMatName != materialName) {
				// Finish earlier group first
				finishCurrentGroup();

				// Update "current" variables
				currentMatName = materialName;
			}
			// Just return the ealier group start index
			return currentObjMatFaceGroupStart;
		}

		/** True if this creator is owning the underlying Obj - false if it has been provided to it! */
		inline bool isOwningObj() {
			return ownsObj;
		}

		/** Returns true if the data in this object is originally once created from an Obj object and we are appending to it! */
		inline bool isCreatedWithBase() {
			return createdWithBase;
		}

	private:
		// Mutable state for groups and materials
		int currentObjMatFaceGroupStart;
		std::string currentGrpName;
		std::string currentMatName;

		// Main representation
		bool ownsObj;
		bool createdWithBase;
		Obj *obj;

		/** Close down the currently opened group */
		inline void finishCurrentGroup() {
			// Lookup currently used material (if there is any)
			TextureDataHoldingMaterial currentMat;
			if(currentMatName != "") {
				// Rem.: Implementation uses "operator[]" so this adds a new empty material
				//       in case a bad name is provided! This is a sensible fallback here!
				currentMat = obj->mtlLib.getNonLoadedMaterialFor(currentMatName);
			}
			// Extend the material face groups with the group we are closing down right now
			obj->extendObjectMaterialGroups(currentGrpName,
							currentMat,
							currentObjMatFaceGroupStart, // this is the old here still!
							obj->fs.size() - currentObjMatFaceGroupStart); // length!
			// Next one will start exactly where we are now
			currentObjMatFaceGroupStart = obj->fs.size();
		}
	};
}

#endif // _OBJMASTER_OBJ_CREATOR_H
