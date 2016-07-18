//
// Created by rthier on 2016.04.12..
//

#ifndef NFTSIMPLEPROJ_OBJMESHOBJECT_H
#define NFTSIMPLEPROJ_OBJMESHOBJECT_H

#include "Obj.h"
#include "FaceElement.h"
#include "VertexStructure.h"
#include <memory>
#include <vector>
#include <stdint.h>

namespace ObjMaster {
    /**
     * A 3d mesh out of a *.OBJ file. This can be used to show the object in a scene as it has proper
     * buffers one can use with most CG rendering methods (like OpenGL).
     */
    class ObjMeshObject {
    public:
        /**
         * Says if the mesh has been initialized or not.
         * This is useful when using non-pointer object and you cannot see
         * if it is "nullptr" in that case but need extremal element to decide caching etc.
         */
        bool inited = false;
        // Render-ready data with inlined vertex attributes. Inlining is usually better for GPU
        // cache hitting.
        std::vector<VertexStructure> vertexData;
        std::vector<uint32_t> indices;

        // Empty constructor
        ObjMeshObject() {};

        /** Create an obj mesh object using all faces available in the given Obj */
        ObjMeshObject(const Obj& obj);

        /**
         * Create and obj mesh object using the explicitly given faces from the given Obj.
         * The parameter variables should be in synch with each other and refer to data from the
         * same *.obj file / same Obj data.
         */
        ObjMeshObject(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount);
    private:
        void creationHelper(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount);
    };
}
#endif //NFTSIMPLEPROJ_OBJMESHOBJECT_H
