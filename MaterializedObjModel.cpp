//
// Created by rthier on 2016.06.20..
//

#include "MaterializedObjModel.h"

namespace ObjMaster {

	MaterializedObjModel::MaterializedObjModel(const Obj &obj) {
		// We create one mesh per object material groups
		for(auto gPair : obj.objectMaterialGroups) {
			printf("asdfasdf\n");
			meshes.push_back(MaterializedObjMeshObject(obj, gPair.second.faces, 1 /*gPair.second.meshFaceCount*/, gPair.second.textureDataHoldingMaterial));
		}

		// Indicate that the model is loaded
		inited = true;
	}
}
