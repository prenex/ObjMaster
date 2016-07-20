//
// Created by rthier on 2016.06.17..
//

#ifndef NFTSIMPLEPROJ_MATERIALIZEDOBJMESHOBJECT_H
#define NFTSIMPLEPROJ_MATERIALIZEDOBJMESHOBJECT_H

#include "TextureDataHoldingMaterial.h"
#include "ObjMeshObject.h"

namespace ObjMaster {
    /**
     * A special ObjMeshObject that has a material associated with it. The mesh can be rendered with
     * that material. The class can be used to group together parts of the obj model that share the
     * same mesh. The template parameter is used to set the library to use for manipulating various
     * texture memories. For the template parameter see TextureDataHoldingMaterial doc-comments!
     */
    class MaterializedObjMeshObject : public ObjMeshObject {
    public:
        /** The corresponding material and possibly the texture data in it */
        TextureDataHoldingMaterial material;
	//TextureDataHoldingMaterial material;

        /** Create an obj mesh-object that is having an associated material */
        MaterializedObjMeshObject(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount, TextureDataHoldingMaterial textureDataHoldingMaterial);
    };
}


#endif //NFTSIMPLEPROJ_MATERIALIZEDOBJMESHOBJECT_H
