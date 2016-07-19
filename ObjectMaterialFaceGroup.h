//
// Created by rthier on 2016.06.21..
//

#ifndef NFTSIMPLEPROJ_MATERIALFACEGROUP_H
#define NFTSIMPLEPROJ_MATERIALFACEGROUP_H

#include "TextureDataHoldingMaterial.h"
#include "NopTexturePreparationLibrary.h"
#include "FaceElement.h"
#include <string>

namespace ObjMaster {
    /**
     * Groups together those faces that share the same material in the same object/group.
     *
     * BEWARE: This class contains bare pointers to the faces of the embedding Obj object so they
     *         have a shared life-cycle and Obj "owns" this object instance then. You are not
     *         entitled to cache a reference to this class or copy it for your own usage!
     */
    struct ObjectMaterialFaceGroup {
        /** The name of the object-group - in case of no group, this becomes the empty string */
        std::string objectGroupName;

        /**
         * The material for the faces in the group - texture data should not be loaded here!
         * We do not own the pointed object, it is owned by the parent object our life-cycle is
         * bound to (for example an object with class Obj or some object with the same life-span)
         */
        TextureDataHoldingMaterial textureDataHoldingMaterial;

        /** Pointer to the first faceElement for this object name and material */
        // TODO: think about bare pointer usage - can we remove it while keeping speed?
        FaceElement *faces;

        /** Defines how many faces are in the group with this material after the pointed address */
        int meshFaceCount;
    };
}

#endif //NFTSIMPLEPROJ_MATERIALFACEGROUP_H
