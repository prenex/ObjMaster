//
// Created by rthier on 2016.06.17..
//

#include "MaterializedObjMeshObject.h"

namespace ObjMaster {

    // Rem.: Copy construction of material here is really okay. It happens only once and on the
    // side of the calling code we will ensure that those materials are not having loaded texture
    // data yet. Without loaded texture data, a copy is not wasting much resources here and saves
    // us from using shared pointers and such things...
    MaterializedObjMeshObject::MaterializedObjMeshObject(const Obj& obj,
                                                         const FaceElement *meshFaces,
                                                         int meshFaceCount,
                                                         TextureDataHoldingMaterial meshObjectMaterial,
                                                         std::string mName)
    : material(meshObjectMaterial), name(mName), ObjMeshObject(obj, meshFaces, meshFaceCount){}
}
