//
// Created by rthier on 2016.04.08..
//

#include "objmasterlog.h"
#include "Obj.h"
#include "UseMtl.h"
#include "ObjectGroupElement.h"
#include <fstream>
#include <memory>
#include <vector>

#define MAT_SEP ":mtl:"
//#define DEBUG 1

namespace ObjMaster {
    Obj::Obj(const AssetLibrary &assetLibrary, const char *path, const char *fileName) {
        constructionHelper(assetLibrary, path, fileName, Obj::EXPECTED_VERTEX_DATA_NUM, Obj::EXPECTED_FACES_NUM);
    }
    Obj::Obj(const AssetLibrary &assetLibrary, const char *path, const char *fileName, int expectedVertexDataNum, int expectedFaceNum) {
        constructionHelper(assetLibrary, path, fileName, expectedVertexDataNum, expectedFaceNum);
    }

    // Helper function for common constructor code-paths
    void Obj::constructionHelper(const AssetLibrary &assetLibrary, const char *path, const char *fileName, int expectedVertexDataNum, int expectedFaceNum) {
	// Save the path of the file that will be opened
	objPath = path;
        OMLOGI("Opening input stream for %s%s", path, fileName);
        std::unique_ptr<std::istream> input = assetLibrary.getAssetStream(path, fileName);

        OMLOGI("Initializing data vectors (expectedVertexDataNum:%d, expectedFaceNum:%d)", expectedVertexDataNum, expectedFaceNum);
        // Initialize vectors with some meaningful default sizes
        vs = std::vector<VertexElement>();
        vs.reserve(expectedVertexDataNum);
        vts = std::vector<VertexTextureElement>();
        vts.reserve(expectedVertexDataNum);
        vns = std::vector<VertexNormalElement>();
        vns.reserve(expectedVertexDataNum);
        fs = std::vector<FaceElement>();
        fs.reserve(expectedFaceNum);

        // We are holding the current material in this variable
        // Can be updated by usemtl descriptors!
        TextureDataHoldingMaterial currentMaterial;
        // We are holding the name of the current object/group here. In case there is no group
        // this can be safely the empty string.
        std::string currentObjectGroupName;
        // This holds the current pointer to the start of the faces in case of the material and/or
        // object grouping...
        int currentObjectMaterialFacesPointer = 0;
        // We hold the pointer to the last of the faces for pointer arithmetics common to the
        // various cases below
        int currentLastFacesPointer = 0;

        // Parse the given file line-by-line
        OMLOGI("Reading obj data file line-by-line");
        char line[DEFAULT_LINE_PARSE_LEN];
        while(input->getline(line, DEFAULT_LINE_PARSE_LEN)) {
            currentLastFacesPointer =( int)fs.size();;
            if(VertexElement::isParsable(line)) {
                // v
                VertexElement v = VertexElement((const char*)line);
                vs.push_back(v);
#ifdef DEBUG
OMLOGE(" - Added VertexElement: (%f, %f, %f)", v.x, v.y, v.z);
#endif
            } else if(VertexTextureElement::isParsable(line)) {
                // vt
                vts.push_back(VertexTextureElement((const char*)line));
            } else if(VertexNormalElement::isParsable(line)) {
                // vn
                vns.push_back(VertexNormalElement((const char*)line));
            } else if(FaceElement::isParsable(line)) {
                // f
                fs.push_back(FaceElement((const char*)line));
            } else if(MtlLib::isParsable(line)) {
                // mtllib
                mtlLib = MtlLib(line, path, assetLibrary);
            } else if(UseMtl::isParsable(line)) {
                // usemtl
                // End the collection of the currentObjectMaterialFaceGroup
                extendObjectMaterialGroups(currentObjectGroupName,
                                           currentMaterial,
                                           currentObjectMaterialFacesPointer,
                                           currentLastFacesPointer - currentObjectMaterialFacesPointer);

                // Rem.: This copy is cheap as it does not contain texture data etc!
                currentMaterial = mtlLib.getNonLoadedMaterialFor(UseMtl::fetchMtlName(line));
#ifdef DEBUG
OMLOGE(" - Using current-material: %s", currentMaterial.name);
#endif
                // Set the current face start pointer to the current position
                // so that the faces will be "collected" for the group
                // BEWARE: This let us overindex the array if no faces are coming!!!
                //         We need to check this overindexint below!
                currentObjectMaterialFacesPointer = (int)fs.size();
            } else if(ObjectGroupElement::isParsable(line)) {
                // o
                // End the collection of the currentObjectMaterialFaceGroup
                extendObjectMaterialGroups(currentObjectGroupName,
                                           currentMaterial,
                                           currentObjectMaterialFacesPointer,
                                           currentLastFacesPointer - currentObjectMaterialFacesPointer);
                currentObjectGroupName = ObjectGroupElement::getObjectGroupName(line);
#ifdef DEBUG
OMLOGE(" - Start of object group: %s", currentObjectGroupName);
#endif
                // Set the current face start pointer to the current position
                // so that the faces will be "collected" for the group
                // BEWARE: This let us overindex the array if no faces are coming!!!
                //         We need to check this overindexint below!
                currentObjectMaterialFacesPointer = (int)fs.size();
            } else {
                OMLOGW("Cannot parse line: %s", line);
            }
        }
        // End the collection of the currentObjectMaterialFaceGroup by extending with the elements
        // of the last obj/material group (and pointer update is necessary here too!)
        currentLastFacesPointer = (int)fs.size();
        extendObjectMaterialGroups(currentObjectGroupName,
                                   currentMaterial,
                                   currentObjectMaterialFacesPointer,
                                   currentLastFacesPointer - currentObjectMaterialFacesPointer);

        OMLOGI("Finished loading of Obj data.");
        OMLOGI(" - Read vertices: %d", (int)vs.size());
        OMLOGI(" - Read vertex-textures: %d", (int)vts.size());
        OMLOGI(" - Read vertex-normals: %d", (int)vns.size());
        OMLOGI(" - Read faces: %d", (int)fs.size());
        OMLOGI(" - Read materials: %d", mtlLib.getMaterialCount());
        OMLOGI(" - Read object/material groups: %d", (int)objectMaterialGroups.size());
    }

    /**
     * Helper method used to extend the material face groups with the given data.
     */
    void Obj::extendObjectMaterialGroups(std::string &currentObjectGroupName,
                                    TextureDataHoldingMaterial &currentMaterial,
                                    int currentObjectMaterialFacesPointer,
                                    int sizeOfFaceStripe) {
        // If the size is zero, we are not saving the group
        // this is not only an optimization, but this is how we handle mtllib ...; o ... after each
        // other (so that we are not creating a lot of empty and unnecessary elements!)
        if (sizeOfFaceStripe > 0) {
            this->objectMaterialGroups[currentObjectGroupName + MAT_SEP + currentMaterial.name]
               = ObjectMaterialFaceGroup {
                    currentObjectGroupName,
                    currentMaterial,
                    currentObjectMaterialFacesPointer,
                    sizeOfFaceStripe
                };
        }
    }
}
