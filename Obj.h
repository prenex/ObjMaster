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
#include "ObjCommon.h"
#include "AssetLibrary.h"
#include "MtlLib.h"
#include <memory>
#include <vector>
#include <unordered_map>

namespace ObjMaster {

    /**
     * Represents a *.obj file. The semantic structure of the object is the same as the file. So
     * the resulting representation after parsing is generally still not feasible for rendering.
     * See ObjMeshObject and similar classes for the rendering possibilities using this model.
     */
    class Obj final {
    public:
        /**
         * Used to initialize the vectors for vertices / normals / texture coordinates for models
         */
        static const int EXPECTED_VERTEX_DATA_NUM = 256;
        /** Used to initialize the vectorfor faces - should be reasonable for usual models */
        static const int EXPECTED_FACES_NUM = 128;

	// Various geometry elements 
        std::vector<VertexElement> vs;
        std::vector<VertexTextureElement> vts;
        std::vector<VertexNormalElement> vns;
        std::vector<FaceElement> fs;

	/** The given path - saved on construction */
	const char* objPath;

        /** The material library for this obj. It can be an empty material library. */
        MtlLib mtlLib;

        /** (objectgroup-name:material-name) -> (objectgroup-name, material, faces) */
        std::unordered_map<std::string, ObjectMaterialFaceGroup> objectMaterialGroups;

	/** Create an empty - non-loaded - obj representation */
	Obj() {}

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

	// Rem.: bit trickery here
	/** Defines the saving mode */
	enum ObjSaveModeFlags{
		/** Only vertices, texcoords and normals plus the faces. No grouping and no materials. */
		ONLY_UNGROUPED_GEOMETRY = 0,
		/** Saving materials - no grouping */
		MATERIALS_GEOMETRY = 1,
		/** Saving 'o' groups - no materials */
		GROUPS_GEOMETRY = 2,
		/** Both materials and 'o' groups */
		MATERIALS_AND_GROUPS = 3,
		// 4) Rem.: bit at position 3 means using 'g' instead of 'o'
		/** !!UNUSED!! */
		G = 4,
		/** Saving 'g' groups - no materials */
		G_GROUPS_GEOMETRY = 2+4,
		/** Both materials and 'g' groups */
		MATERIALS_AND_G_GROUPS = 3+4,
	};

	/** Save this Obj as a (relative) *.obj - using the given fileName and the provided asset-out library. By default this also saves the *.mtl */
	inline void saveAs(const AssetOutputLibrary &assetOutputLibrary, const char* fileName, bool saveAsMtlToo = true, ObjSaveModeFlags saveMode = ObjSaveModeFlags::MATERIALS_AND_GROUPS){
		// Just delegate to the real method
		saveAs(assetOutputLibrary, "", fileName, saveAsMtlToo, saveMode);
	}

	/** Save this Obj as an (absolute) *.obj - using the given path, fileName and the provided asset-out library. By default this also saves the *.mtl */
	void saveAs(const AssetOutputLibrary &assetOutputLibrary, const char* path, const char* fileName, bool saveAsMtlToo = true, ObjSaveModeFlags saveMode = ObjSaveModeFlags::MATERIALS_AND_GROUPS);
    private:
        void constructionHelper(const AssetLibrary &assetLibrary,
                            const char *path, const char *fileName,
                            int expectedVertexDataNum, int expectedFaceNum);

        /**
         * Helper method used to extend the material face groups with the given data.
         */
        void extendObjectMaterialGroups(std::string &currentObjectGroupName,
                                        TextureDataHoldingMaterial &currentMaterial,
                                        int currentObjectMaterialFacesPointer,
                                        int sizeOfFaceStripe);
    };

    /** Run tests for underlying elements */
    static bool TEST_Obj() {
#ifdef DEBUG
        OMLOGI("TEST_Obj...");
#endif
        bool res = true;
        res &= ObjMaster::TEST_VertexElement();
        res &= ObjMaster::TEST_VertexNormalElement();
        res &= ObjMaster::TEST_VertexTextureElement();
        res &= ObjMaster::TEST_FaceElement();
        res &= ObjMaster::TEST_FacePoint();
#ifdef DEBUG
        OMLOGI("...TEST_Obj ended!");
#endif
        return res;
    }
}

#endif //NFTSIMPLEPROJ_OBJ_CPP_H
