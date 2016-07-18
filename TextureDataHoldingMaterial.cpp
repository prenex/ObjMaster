//
// Created by rthier on 2016.06.20..
//

#include "TextureDataHoldingMaterial.h"

namespace ObjMaster {

    // Rem.: Most of the implementation is in the header file, because of templating...
    TextureDataHoldingMaterial::TextureDataHoldingMaterial(std::string materialName,
                                                           std::vector<std::string> descriptorLineFields)
            : Material(materialName, descriptorLineFields) {
    }

}
