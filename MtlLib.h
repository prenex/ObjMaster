//
// Created by rthier on 2016.06.21..
//

#ifndef NFTSIMPLEPROJ_MTLLIB_H
#define NFTSIMPLEPROJ_MTLLIB_H

#include "TextureDataHoldingMaterial.h"
#include "AssetLibrary.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace ObjMaster {
    class MtlLib {
    public:
        static const std::string KEYWORD;
        /**
         * !ONLY USE IF YOU KNOW WHAT YOU ARE DOING! Contains the names of the material library files that this library builds upon.
         */
        // TODO: In the *.obj file, they can be there as a CSV - currently not supported when reading *.obj files!
        std::vector<std::string> libraryFiles;

        /** Decides if the given fields can be parsed as an MtlLib element */
        static bool isParsable(const char *fields);

        /**
         * Create a material library from the given obj fields. The constructor also load the mtl
         * file to construct the whole corresponding library thus the asses library is needed.
         * The given asset library is only used for construction and is not stored so the ownership
         * of the referenced object stays at the host code!
         */
        MtlLib(const char *fields, const char *assetPath, const AssetLibrary &assetLibrary);
        /**
         * Create a material library from the given obj fields. The constructor also load the mtl
         * file to construct the whole corresponding library thus the asses library is needed.
         * The given asset library is only used for construction and is not stored so the ownership
         * of the referenced object stays at the host code!
         */
        MtlLib(char *fields, const char *assetPath, const AssetLibrary &assetLibrary);
        /** The default constructor just create a completely empty library */
        MtlLib() {}

	// Copies are defeaulted
	MtlLib(const MtlLib &other) = default;
	MtlLib& operator=(const MtlLib &other) = default;
	// Moves are defaulted
	MtlLib(MtlLib &&other) = default;
	MtlLib& operator=(MtlLib &&other) = default;

	/** The absolute textual reference to the *.mtl as it is references from the *.obj file - this should go into the *.obj output! */
	inline std::string asText(const char *path, const char *fileName) {
		// Rem.: The compiler should be smart-enough to optimize this when inlined...
		std::string fullPath = std::string(path) + fileName;
		return asText(fullPath.c_str());
	}
	/** The relative textual reference to the *.mtl as it is references from the *.obj file - this should go into the *.obj output! */
	inline std::string asText() {
		// Rem.: This is not that much suboptimal as in most cases there is only one *.mtl
		std::string allLibraryFiles = "";
		bool first = true;
		for(int i = 0; i < libraryFiles.size(); ++i) {
			// Add separating spaces
			if(first) {
				first = false;
			}else{
				allLibraryFiles+=' ';
			}
			// Add library file to the concatenated string
			allLibraryFiles+=libraryFiles[i];
		}
		return asText(allLibraryFiles.c_str());
	}

	/** The relative textual reference to the *.mtl as it is references from the *.obj file - this should go into the *.obj output! */
	inline static std::string asText(const char *fileName) {
		return KEYWORD + " " + fileName;
	}

	/** Save this MtlLib as a *.mtl - using the (relative) fileName and the provided asset-out library */
	inline void saveAs(const AssetOutputLibrary &assetOutputLibrary, const char* fileName) {
		saveAs(assetOutputLibrary, "", fileName);
	}
	/** Save this MtlLib as a *.mtl - using the path, fileName and the provided asset-out library */
	void saveAs(const AssetOutputLibrary &assetOutputLibrary, const char* path, const char* fileName, bool alwaysGrowLibraryFilesList = false);

	/** Adds the given (runtime generated) material to the material library. If there is a material with the same name, it gets overwritten! */
	inline void addRuntimeGeneratedMaterial(Material m) {
		// Convert the provided to an unloaded TextureDataHoldingMaterial...
		materials[m.name] = TextureDataHoldingMaterial(m);
	}

        /**
	 * Returns a copy of the material with the given name - this material is always non-loaded!
	 * Rem.: Implementation uses "operator[]" so this adds a new empty material to the MtlLib
	 *       in case a bad name is provided! This is usually a sensible fallback - but beware!
	 */
	inline TextureDataHoldingMaterial getNonLoadedMaterialFor(std::string materialName) {
		// Just return the material for the name
		return materials[materialName];
	}

	/** Gets all material names that are currently stored in the material library */
	inline std::vector<std::string> getAllMaterialNames() {
		std::vector<std::string> ret;
		for(auto kv : materials) {
			ret.push_back(kv.first);
		}
		return ret;
	}

        /** Returns the number of materials in this library */
        int getMaterialCount();

        /** Returns if this mtllib is a completely empty library or not! */
        bool isEmpty() { return materials.empty(); }
    private:

        /** Material name -> material hash for basic material access without loaded texture data */
        std::unordered_map <std::string, TextureDataHoldingMaterial> materials;

        void constructionHelper(char *fields, const char *assetPath, const AssetLibrary &assetLibrary);

        /** !!! Helper function - BEWARE: This might change the argument as a side-effect !!! */
        std::string updateCurrentMaterialName(char *line);
    };
}

#endif //NFTSIMPLEPROJ_MTLLIB_H
