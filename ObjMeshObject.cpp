//
// Created by rthier on 2016.04.12..
//

// Un-comment in case of hard debugging
#define DEBUG

#include "objmasterlog.h"
#include "ObjMeshObject.h"
#include <memory>
#include <unordered_map> // for hashing
#include <vector> // for list-handling
#include "VertexStructure.h"

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
        return lhs.v->x == rhs.v->x &&
               lhs.v->y == rhs.v->y &&
               lhs.v->z == rhs.v->z &&
               lhs.vn->x == rhs.vn->x &&
               lhs.vn->y == rhs.vn->y &&
               lhs.vn->z == rhs.vn->z &&
               lhs.vt->u == rhs.vt->u &&
               lhs.vt->v == rhs.vt->v;
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
            result_type hv1 = std::hash<float>()(s.v->x);
            result_type hv2 = std::hash<float>()(s.v->y);
            result_type hv3 = std::hash<float>()(s.v->z);

            result_type hvn1 = std::hash<float>()(s.vn->x);
            result_type hvn2 = std::hash<float>()(s.vn->y);
            result_type hvn3 = std::hash<float>()(s.vn->z);

            result_type hvt1 = std::hash<unsigned int>()(s.vt->u);
            result_type hvt2 = std::hash<unsigned int>()(s.vt->v);

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
        creationHelper(obj, objFaces, objFaceCount);
    }

    ObjMeshObject::ObjMeshObject(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount) {
        creationHelper(obj, meshFaces, meshFaceCount);
    }

    void ObjMeshObject::creationHelper(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount) {
        // The map is used to make the index buffer refer to duplications properly
        // without re-creating the data slice for the duplications
        std::unordered_map<IndexTargetSlice, uint32_t> alreadyHandledFacePointTargets;

        // Vectors are used here as we are duplicating and/or filtering out data
        // so we do not know for sure if we have this or that many elements
        indices = std::vector<uint32_t>();

        // Inline data for better GPU cache hit! This way we need several shader attributes with one array!
        vertexData = std::vector<VertexStructure>();

        // The reservations here are really just heuristics:
        // - It would be pointless to think the indices always point at different things
        // - In that case it would be 3*mfc (considering triangles)
        // - So what I did is that I just heuristically applied one third of those maximums
        if(meshFaceCount != 0) {
            indices.reserve(meshFaceCount);
            vertexData.reserve(meshFaceCount);
        }

        // Loop through faces
        uint32_t lastIndex = 0;
        for(int i = 0; i < meshFaceCount; ++i) {
            FaceElement face = meshFaces[i];
            if(face.facePointCount == 3) {
                // Loop through all points in faces
                for(int j = 0; j < face.facePointCount; ++j) {
                    FacePoint fp = face.facePoints[j];
                    // Create pointers to the target data of the face-point
                    // (This should be faster than copy)
                    const VertexElement *fv = &obj.vs[fp.vIndex];
                    const VertexTextureElement *fvt = &obj.vts[fp.vtIndex];
                    const VertexNormalElement *fvn = &obj.vns[fp.vnIndex];
#ifdef DEBUG
OMLOGD("Processing face:");
OMLOGD(" - vIndex: %d", fp.vIndex);
OMLOGD(" - vtIndex: %d", fp.vtIndex);
OMLOGD(" - vnIndex: %d", fp.vnIndex);
OMLOGD("with:");
OMLOGD(" - vs[vIndex]: (%f, %f, %f)", obj.vs[fp.vIndex].x, obj.vs[fp.vIndex].y, obj.vs[fp.vIndex].z);
OMLOGD(" - vts[vtIndex]: (%f, %f)", obj.vts[fp.vtIndex].u, obj.vts[fp.vtIndex].v);
OMLOGD(" - vns[vnIndex]: (%f, %f, %f)", obj.vns[fp.vnIndex].x, obj.vns[fp.vnIndex].y, obj.vns[fp.vnIndex].z);
#endif

                    // Create a target slice from the target data
                    // This slicer is only used for hashing out duplications. Ownership of data
                    // is not transferred as this is a read-only operation!
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
                        indices.push_back(alreadyHandledFacePointTargets[its]);
                    } else {
                        // Collect target data in lists that represent the buffers
                        // Basically add the data variation for the vertical slice

                        vertexData.push_back((VertexStructure) {
                                fv->x,
                                fv->y,
                                fv->z,
                                fvn->x,
                                fvn->y,
                                fvn->z,
                                fvt->u,
                                fvt->v});
                        // Add an index for this new vertical slice
                        indices.push_back(lastIndex);

                        // Update the hashmap for the already handled data
                        alreadyHandledFacePointTargets[its] = lastIndex;

                        // Increment the index-buffer construction variable
                        ++lastIndex;
                    }
                }
            } else {
                // TODO: Should we handle non-triangles ever?
                OMLOGE(" - Found a face that has more than 3 vertices(%d) - skipping face!", face.facePointCount);
            }
        }

        // Log relevant counts
        OMLOGD(" - Number of vertices after conversion: %d", (int)vertexData.size());
        OMLOGD(" - Number of indices after conversion: %d", (int)indices.size());
        OMLOGD(" - Maximum indexNo: %d", lastIndex);

        // Say that the mesh has been initialized
        inited = true;
    }
}
