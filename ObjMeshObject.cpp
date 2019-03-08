//
// Created by rthier on 2016.04.12..
// TODO: In multi-mesh cases the indexing is suboptimal as there can be duplications between meshes (only when they reuse vertices between two mesh - not horrible)

// Un-comment in case of hard debugging
/*#define DEBUG*/

#include "objmasterlog.h"
#include "ObjMeshObject.h"
#include <memory>
#include <unordered_map> // for hashing
#include <vector> // for list-handling
#include "VertexStructure.h"
#include <algorithm> // for std::swap

// Used as key for hashing
struct IndexTargetSlice {
    IndexTargetSlice(const ObjMaster::VertexElement *v_,
                     const ObjMaster::VertexTextureElement *vt_,
                     const ObjMaster::VertexNormalElement *vn_) {
        v = v_;
        vt = vt_;
        vn = vn_;
    }

    // We are using pointers here just to use less memory. See implementation below.
    // These pointers only refer to one element each!!!
    const ObjMaster::VertexElement *v;
    const ObjMaster::VertexTextureElement *vt;
    const ObjMaster::VertexNormalElement *vn;

    // Necessary for hashing
    friend bool operator==(const IndexTargetSlice& lhs, const IndexTargetSlice& rhs) {
        bool vBool;
	if(lhs.v != nullptr && rhs.v != nullptr) {
		vBool = lhs.v->x == rhs.v->x &&
		       lhs.v->y == rhs.v->y &&
		       lhs.v->z == rhs.v->z;
	} else {
		// Handle case when there is no vertex coordinate (weird case)
		vBool = (lhs.v == rhs.v);
	}

        bool vtBool;
	if(lhs.vt != nullptr && rhs.vt != nullptr) {
		vtBool = lhs.vt->u == rhs.vt->u &&
		       lhs.vt->v == rhs.vt->v;
	} else {
		// Handle case when there is no vertex coordinate (many cases)
		vtBool = (lhs.vt == rhs.vt);
	}

        bool vnBool;
	if(lhs.vn != nullptr && rhs.vn != nullptr) {
		vnBool = lhs.vn->x == rhs.vn->x &&
		       lhs.vn->y == rhs.vn->y &&
		       lhs.vn->z == rhs.vn->z;
	} else {
		// Handle case when there is no vertex normal (possible cases)
		vnBool = (lhs.vn == rhs.vn);
	}


	return vBool && vnBool && vtBool;
    }
};

// Custom specialization of std::hash can be injected in namespace std
namespace std
{
    template<> struct hash<IndexTargetSlice>
    {
        typedef IndexTargetSlice argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            result_type hv1 = 0;
            result_type hv2 = 0;
            result_type hv3 = 0;
	    if(s.v != nullptr) {
		    hv1 = std::hash<float>()(s.v->x);
		    hv2 = std::hash<float>()(s.v->y);
		    hv3 = std::hash<float>()(s.v->z);
	    }

            result_type hvn1 = 0;
            result_type hvn2 = 0;
            result_type hvn3 = 0;
	    if(s.vn != nullptr) {
		    hvn1 = std::hash<float>()(s.vn->x);
		    hvn2 = std::hash<float>()(s.vn->y);
		    hvn3 = std::hash<float>()(s.vn->z);
	    }

            result_type hvt1 = 0;
            result_type hvt2 = 0;
	    if(s.vt != nullptr) {
		    hvt1 = std::hash<float>()(s.vt->u);
		    hvt2 = std::hash<float>()(s.vt->v);
	    }

            return (hv1 ^ (hv2 << 1)) ^ (hv3 << 1) ^
                   ((hvn1 ^ (hvn2 << 1)) ^ (hvn3 << 1)) ^
                   ((hvt1) ^ (hvt2 << 1));
        }
    };
}


namespace ObjMaster {

    ObjMeshObject::ObjMeshObject(const Obj& obj) {
        // According to the standard, vector elements are places in the memory after each other!
        // This way we can create a c-style array/pointer by referring to the address to the first!
        const FaceElement* objFaces = &(obj.fs)[0];
        // The count of mesh faces should be equal to all of the faces in this case
        int objFaceCount = (obj.fs).size();
        creationHelper(obj, objFaces, objFaceCount, nullptr, nullptr, 0);
    }

    ObjMeshObject::ObjMeshObject(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount) {
        creationHelper(obj, meshFaces, meshFaceCount, nullptr, nullptr, 0);
    }

	/**
	 * The copy ctor - necessary because of the possible pointer sharing stuff!
	 * In case of non-shared vectors we copy, in case of shared vectors we copy only the pointer!
	 */
	ObjMeshObject::ObjMeshObject(const ObjMeshObject &other) {
		copyHelper(other);
	}

	/**
	 * The copy ctor - necessary because of the possible pointer sharing stuff!
	 * In case of non-shared vectors we copy, in case of shared vectors we copy only the pointer!
	 */
	ObjMeshObject& ObjMeshObject::operator=(const ObjMeshObject &other) {
		copyHelper(other);
		return *this;
	}

	/**
	 * The move ctor - necessary because of the possible pointer sharing stuff!
	 * In case of non-shared vectors we move, in case of shared vectors we move only the pointer!
	 */
	ObjMeshObject::ObjMeshObject(ObjMeshObject &&other) {
		moveHelper(std::move(other));
	}

	/**
	 * The move ctor - necessary because of the possible pointer sharing stuff!
	 * In case of non-shared vectors we move, in case of shared vectors we move only the pointer!
	 */
	ObjMeshObject& ObjMeshObject::operator=(ObjMeshObject &&other) {
		moveHelper(std::move(other));
		return *this;
	}

	void ObjMeshObject::moveHelper(ObjMeshObject &&other) {
		// In case of a move, we can just move everything
		std::swap(this->baseVertexLocation, other.baseVertexLocation);
		std::swap(this->indexCount, other.indexCount);
		std::swap(this->indices, other.indices);
		std::swap(this->lastIndex, other.lastIndex);
		std::swap(this->ownsIndices, other.ownsIndices);
		std::swap(this->ownsVertexData, other.ownsVertexData);
		std::swap(this->startIndexLocation, other.startIndexLocation);
		std::swap(this->vertexCount, other.vertexCount);
		std::swap(this->vertexData, other.vertexData);
		std::swap(this->inited, other.inited);

		// But ensure that the "other" thinks he does not own anything anymore!
		// This is necessary because we might have got ownership and other should not delete pointers then!
		other.ownsIndices = false;
		other.ownsVertexData = false;
	}
	void ObjMeshObject::copyHelper(const ObjMeshObject &other) {
		// The pointers to vectors are copied according to the ownership flags!
		std::vector<OM_INDEX_TYPE> *iPtr = nullptr;
		std::vector<VertexStructure> *vPtr = nullptr;
		if (other.ownsIndices) {
			// If the other owns the pointed vector
			// we should copy its contents and create
			// a vector that we own by ourselves.
			// There are more semantic possibilities, but
			// this handles a lot of common cases and
			// ensures that the desctuctor does not tries
			// to delete memory that is not owned by us!
			iPtr = new std::vector<OM_INDEX_TYPE>(*other.indices);
		} else {
			// If the other do not own the pointed vector
			// we can just use a pointer to it and we also
			// don't own the vector
			iPtr = other.indices;
		}
		if (other.ownsVertexData) {
			// If the other owns the pointed vector
			// we should copy its contents and create
			// a vector that we own by ourselves.
			// There are more semantic possibilities, but
			// this handles a lot of common cases and
			// ensures that the desctuctor does not tries
			// to delete memory that is not owned by us!
			vPtr = new std::vector<VertexStructure>(*other.vertexData);
		} else {
			// If the other do not own the pointed vector
			// we can just use a pointer to it and we also
			// don't own the vector
			vPtr = other.vertexData;
		}

		// Other things are just copied memberwise
		this->baseVertexLocation = other.baseVertexLocation;
		this->indexCount = other.indexCount;
		this->indices = iPtr;
		this->lastIndex = other.lastIndex;
		this->ownsIndices = other.ownsIndices;
		this->ownsVertexData = other.ownsVertexData;
		this->startIndexLocation = other.startIndexLocation;
		this->vertexCount = other.vertexCount;
		this->vertexData = vPtr;
		this->inited = other.inited;
	}

	ObjMeshObject::ObjMeshObject(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount, std::vector<VertexStructure> *vertexVector, std::vector<OM_INDEX_TYPE> *indexVector, OM_INDEX_TYPE lastIndexBase) {
		creationHelper(obj, meshFaces, meshFaceCount, vertexVector, indexVector, lastIndexBase);
	}

    void ObjMeshObject::creationHelper(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount,
		std::vector<VertexStructure> *vertexVector, std::vector<OM_INDEX_TYPE> *indexVector, OM_INDEX_TYPE lastIndexBase) {

		// Handle the difference between the case when they provide the vectors to us
		// and cases when we create and own the vectors by ourselves!

		// Vertex vector
		if (vertexVector == nullptr) {
			this->vertexData = new std::vector<VertexStructure>();
			this->ownsVertexData = true;
			this->baseVertexLocation = 0;
		} else {
			this->vertexData = vertexVector;
			this->ownsVertexData= false;
			this->baseVertexLocation = vertexVector->size();
		}

		// Index vector
		if (indexVector == nullptr) {
			this->indices = new std::vector<OM_INDEX_TYPE>();
			this->ownsIndices = true;
			this->startIndexLocation = 0;
		}
		else {
			this->indices = indexVector;
			this->ownsIndices = false;
			this->startIndexLocation = indexVector->size();
		}

        // The map is used to make the index buffer refer to duplications properly
        // without re-creating the data slice for the duplications
        std::unordered_map<IndexTargetSlice, OM_INDEX_TYPE> alreadyHandledFacePointTargets;

        // The reservations here are really just heuristics:
        // - It would be pointless to think the indices always point at different things
        // - In that case it would be 3*mfc (considering triangles)
        // - So what I did is that I just heuristically applied one third of those maximums
        if(meshFaceCount != 0) {
			// Here the earlier sizes should be added
			// as the parameter to reserve is an absolute
			// reservation size and in case of shared
			// vectors they are already having some reserves!
			//
			// In complex cases this also helps to reduce the
			// over-reservations as we only over-reserve by the
			// amount of quessing error from the last mesh!!!
			// This is why we use xxx.size() as base here!!!
            indices->reserve(indices->size() + meshFaceCount);
            vertexData->reserve(vertexData->size() + meshFaceCount);
        }

        // Loop through faces
		indexCount = vertexCount = 0;	// In mesh counts: zero
        lastIndex = lastIndexBase;		// Multimesh: avoids crash with earlier indices!
        for(int i = 0; i < meshFaceCount; ++i) {
            FaceElement face = meshFaces[i];
            if(face.facePointCount == 3) {
                // Loop through all points in faces
                for(int j = 0; j < face.facePointCount; ++j) {
                    FacePoint fp = face.facePoints[j];
                    // Create pointers to the target data of the face-point
                    // (This should be faster than copy)
					// -1 indicates a missing element so we handle it as if there is one!
					// Rem.: The real representation type is unsigned so basically the special value is not -1, but the unsigned int max value!
					//       because of this is why we are casting the value to unsigned int. Just to be sure we that it happens as we imagine!
                    const VertexElement *fv = (fp.vIndex != (unsigned int)(-1) ? &obj.vs[fp.vIndex] : nullptr);
                    const VertexTextureElement *fvt = (fp.vtIndex != (unsigned int)(-1) ? &obj.vts[fp.vtIndex] : nullptr);
                    const VertexNormalElement *fvn = (fp.vnIndex != (unsigned int)(-1) ? &obj.vns[fp.vnIndex] : nullptr);
#ifdef DEBUG
OMLOGD("Processing face:");
OMLOGD(" - vIndex: %d", fp.vIndex);
OMLOGD(" - vtIndex: %d", fp.vtIndex);
OMLOGD(" - vnIndex: %d", fp.vnIndex);
OMLOGD("with:");
if(fv != nullptr) { OMLOGD(" - vs[vIndex]: (%f, %f, %f)", obj.vs[fp.vIndex].x, obj.vs[fp.vIndex].y, obj.vs[fp.vIndex].z); }
if(fvt != nullptr) { OMLOGD(" - vts[vtIndex]: (%f, %f)", obj.vts[fp.vtIndex].u, obj.vts[fp.vtIndex].v); }
if(fvn != nullptr) { OMLOGD(" - vns[vnIndex]: (%f, %f, %f)", obj.vns[fp.vnIndex].x, obj.vns[fp.vnIndex].y, obj.vns[fp.vnIndex].z); }
#endif
                    // Create a target slice from the target data
                    // This slicer is only used for hashing out duplications. Ownership of data
                    // is not transferred as this is a read-only operation!
		    // Rem.: the slice also handle nullptrs for optional elements!
                    IndexTargetSlice its = IndexTargetSlice(fv, fvt, fvn);

                    // See if the data for this face-point can be found among the earlier ones
                    if(alreadyHandledFacePointTargets.find(its)
                           != alreadyHandledFacePointTargets.end()) {
#ifdef DEBUG
OMLOGD(" - Found already handled facePoint!");
#endif
                        // Only add a new index into the index-buffer referencing the already
                        // added data in case we had this variation earlier... The index points to
                        // the earlier variation this way.
                        indices->push_back(alreadyHandledFacePointTargets[its]);
						++indexCount;
                    } else {
                        // Collect target data in lists that represent the buffers
                        // Basically add the data variation for the vertical slice
			// Rem.: When position, normal or uv data is missing, we provide
			// some default value here instead of just crashing...
                        vertexData->push_back(VertexStructure {
                                fv != nullptr ? fv->x : 0,
                                fv != nullptr ? fv->y : 0,
                                fv != nullptr ? fv->z : 0,
                                fvn != nullptr ? fvn->x : 0,
                                fvn != nullptr ? fvn->y : 0,
                                fvn != nullptr ? fvn->z : 0,
                                fvt != nullptr ? fvt->u : 0,
                                fvt != nullptr ? fvt->v : 0});
			++vertexCount;

                        // Add an index for this new vertical slice
                        indices->push_back(lastIndex);
			++indexCount;

                        // Update the hashmap for the already handled data
                        alreadyHandledFacePointTargets[its] = lastIndex;

                        // Increment the index-buffer construction variable
						// invariant: this always holds max(indices)
                        ++lastIndex;
                    }
                }
            } else {
                // TODO: Should we handle non-triangles ever?
                OMLOGE(" - Found a face that has more than 3 vertices(%d) - skipping face!", face.facePointCount);
            }
        }

        // Log relevant counts
        OMLOGD(" - Total number of vertices in buffer after obj->mesh conversion: %u", ((unsigned int) vertexData->size()));
        OMLOGD(" - Total number of indices in buffer after obj->mesh conversion: %u", ((unsigned int) indices->size()));
        OMLOGD(" - Number of (per-mesh) vertices after conversion: %u", vertexCount);
        OMLOGD(" - Number of (per-mesh) indices after conversion: %u", indexCount);
        OMLOGD(" - Maximum indexNo in this mesh: %d", lastIndex);

        // Indicate that the mesh has been initialized
        inited = true;
    }
}
