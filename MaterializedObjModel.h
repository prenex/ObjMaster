//
// Created by rthier on 2016.06.20..
//

#ifndef NFTSIMPLEPROJ_MATERIALIZEDOBJMODEL_H
#define NFTSIMPLEPROJ_MATERIALIZEDOBJMODEL_H

#include <vector>
#include "MaterializedObjMeshObject.h"

namespace ObjMaster {

    template<class TexturePreparationLibrary>
    class MaterializedObjModel {
    public:
        std::vector<MaterializedObjMeshObject<TexturePreparationLibrary>> meshes;

        /** Create a materialized obj model */
        MaterializedObjModel(Obj obj, std::string texturePath);
    };
}


#endif //NFTSIMPLEPROJ_MATERIALIZEDOBJMODEL_H
