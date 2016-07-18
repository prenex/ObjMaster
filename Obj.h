//
// Created by rthier on 2016.04.08..
//

#ifndef NFTSIMPLEPROJ_OBJ_CPP_H
#define NFTSIMPLEPROJ_OBJ_CPP_H

#include "ObjectMaterialFaceGroup.h"
#include "VertexElement.h"
#include "VertexTextureElement.h"
#include "VertexNormalElement.h"
#include "FaceElement.h"
#include "AssetLibrary.h"
#include "MtlLib.h"
#include <memory>
#include <vector>
#include <unordered_map>

namespace ObjMaster {
    static char const *OBJ_DELIMITER = " ";

    /**
     * Represents a *.obj file. The semantic structure of the object is the same as the file. So
     * the resulting representation after parsing is generally still not feasible for rendering.
     * See ObjMeshObject and similar classes for the rendering possibilities using this model.
     */
    class Obj final {
    public:
        /** This is the maximum line length used for the line-by-line parsing */
        static const int DEFAULT_LINE_PARSE_LEN = 256;
        /**
         * Used to initialize the vectors for vertices / normals / texture coordinates for models
         */
        static const int EXPECTED_VERTEX_DATA_NUM = 256;
        /** Used to initialize the vectorfor faces - should be reasonable for usual models */
        static const int EXPECTED_FACES_NUM = 128;

        std::vector<VertexElement> vs;
        std::vector<VertexTextureElement> vts;
        std::vector<VertexNormalElement> vns;
        std::vector<FaceElement> fs;

        /** The material library for this obj. It can be an empty material library. */
        MtlLib mtlLib;

        /** (objectgroup-name:material-name) -> (objectgroup-name, material, faces) */
        std::unordered_map<std::string, ObjectMaterialFaceGroup> objectMaterialGroups;

        /**
         * Creates an Obj for the given asset. The given asset library is only used for construction
         * and is not stored so the ownership of the referenced object stays at the host code!
         */
        Obj(const AssetLibrary &assetLibrary, const char* path, const char* fileName);

        /**
         * Creates an Obj for the given asset. The given vertex and face data
         * numbers are used when parsing the file but the object will be constructed even when
         * the file contains more data than the expectations! These values are used to instrument
         * the default allocations so it is helpful when optimizing for memory or speed.
         * The given asset library is only used for construction and is not stored so the ownership
         * of the referenced object stays at the host code!
         */
        Obj(const AssetLibrary &assetLibrary, const char *path, const char *fileName,
            int expectedVertexDataNum, int expectedFaceNum);
    private:
        void constructionHelper(const AssetLibrary &assetLibrary,
                            const char *path, const char *fileName,
                            int expectedVertexDataNum, int expectedFaceNum);

        /**
         * Helper method used to extend the material face groups with the given data.
         */
        void extendObjectMaterialGroups(std::string &currentObjectGroupName,
                                        TextureDataHoldingMaterial &currentMaterial,
                                        FaceElement* currentObjectMaterialFacesPointer,
                                        int sizeOfFaceStripe);
    };

    /** Run tests for underlying elements */
    static bool TEST_Obj() {
#ifdef DEBUG
        OMLOGE("TEST_Obj...");
#endif
        bool res = true;
        res &= ObjMaster::TEST_VertexElement();
        res &= ObjMaster::TEST_VertexNormalElement();
        res &= ObjMaster::TEST_VertexTextureElement();
        res &= ObjMaster::TEST_FaceElement();
        res &= ObjMaster::TEST_FacePoint();
#ifdef DEBUG
        OMLOGE("...TEST_Obj ended!");
#endif
        return res;
    }
}

#endif //NFTSIMPLEPROJ_OBJ_CPP_H
