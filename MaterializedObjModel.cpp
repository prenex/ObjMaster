//
// Created by rthier on 2016.06.20..
//

#include "MaterializedObjModel.h"

namespace ObjMaster {

	MaterializedObjModel::MaterializedObjModel(Obj obj) {
		// We create one mesh per object material groups
		for(auto gPair : obj.objectMaterialGroups) {
			meshes.push_back(MaterializedObjMeshObject(obj, gPair.second.faces, gPair.second.meshFaceCount, gPair.second.textureDataHoldingMaterial));
		}

		// Indicate that the model is loaded
		inited = true;
	}
}
