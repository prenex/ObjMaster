//
// Created by rthier on 2016.06.20..
//

#include "MaterializedObjModel.h"

namespace ObjMaster {
	MaterializedObjModel::MaterializedObjModel(const Obj &obj) {
		// The path of the model is the same as the path for obj
		path = obj.objPath;
		// We create one mesh per each object material group
		for(auto gPair : obj.objectMaterialGroups) {
			meshes.push_back(MaterializedObjMeshObject(obj,
				&(obj.fs[gPair.second.faceIndex]),
			       	gPair.second.meshFaceCount,
			       	gPair.second.textureDataHoldingMaterial));
		}

		// Indicate that the model is loaded
		inited = true;
	}
}
