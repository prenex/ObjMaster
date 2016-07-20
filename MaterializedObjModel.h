//
// Created by rthier on 2016.06.20..
//

#ifndef NFTSIMPLEPROJ_MATERIALIZEDOBJMODEL_H
#define NFTSIMPLEPROJ_MATERIALIZEDOBJMODEL_H

#include <vector>
#include "MaterializedObjMeshObject.h"
#include "Obj.h"

namespace ObjMaster {

    class MaterializedObjModel {
    public:
	bool inited = false;
	std::vector<MaterializedObjMeshObject> meshes;

	/** Create a materialized obj model using the given obj representation */
	MaterializedObjModel(const Obj &obj);
	/** Create a materialized obj model that is not inited (empty) */
	MaterializedObjModel() {}
    };
}


#endif //NFTSIMPLEPROJ_MATERIALIZEDOBJMODEL_H
