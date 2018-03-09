//
// Created by rthier on 2016.06.21..
//

#include "funhelper.h"
#include "MtlLib.h"
#include "ObjCommon.h"
#include <algorithm>  /* std::mismatch */
#include <cstring>    /* strtok_r, strdup */
#include <cstdlib>    /* free */
#include <iostream>   /* readLine, << and write calls */
#include "wincompat.h" // msvc hax

namespace ObjMaster {
    const std::string MtlLib::KEYWORD = std::string("mtllib");

    bool MtlLib::isParsable(const char *fields) {
        // First check this: not an empty string, the first character is an 'm'
        if((fields != nullptr)
               && (fields[0] != 0)
               && ((fields[0] == 'm'))){
            // Now check if it has the prefix of mtllib or not
            const std::string targetString = std::string(fields);
            return isStartsWith(targetString, KEYWORD);
        } else {
            return false;
        }
    }

    MtlLib::MtlLib(const char *fields, const char *assetPath, const AssetLibrary &assetLibrary) {
        // The strtok_r changes the string so we need to duplicate it
        char *copy = strdup(fields);
        constructionHelper(copy, assetPath, std::move(assetLibrary));
        free(copy);
    }

    // This variant modifies the provided 'string' but is it faster as it do this without the copy
    MtlLib::MtlLib(char *fields, const char *assetPath, const AssetLibrary &assetLibrary) {
        constructionHelper(fields, assetPath, std::move(assetLibrary));
    }

    void MtlLib::constructionHelper(char *fields, const char *assetPath, const AssetLibrary &assetLibrary) {
        if (isParsable(fields)) {
            char *savePtr;
            char *key = strtok_r(fields, OBJ_DELIMITER, &savePtr);
            char *libFileCstr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);
            // TODO: currently we only support one material library file... should split with ','
            // Also the below code should be a loop through all the files then...
            std::string libraryFile(libFileCstr);
            libraryFiles.push_back(libraryFile);

            OMLOGI("Opening input stream for %s/%s", assetPath, libraryFile.c_str());
            std::unique_ptr<std::istream> input = assetLibrary.getAssetStream(assetPath, libraryFile.c_str());

            OMLOGI("Reading mtl data file line-by-line");
            char line[DEFAULT_LINE_PARSE_LEN];
            std::vector<std::string> descriptorLineFields;
            bool firstMaterial = true;
            std::string currentMaterialName;
            while(input->getline(line, DEFAULT_LINE_PARSE_LEN)) {
                // See if we have found a new material descriptor
                if((line[0] == 'n') && isStartsWith(std::string(line), "newmtl")){
                    // A new material descriptor will start... process data found until now!
                    // If there was a material that we've already started to collect
                    // Then finding the next one means we have all the data for parsing
                    // the current one so we should try creating this one.
                    if(firstMaterial) {
                        // In case this is the first newmtl entry, we can't create the material
                        // object yet as the data for it are still coming to us!
                        firstMaterial = false;
                    } else {
                        // Create a material with all the collected data from the last newmtl entry
                        // and add this to the material mapping
                        //TextureDataHoldingMaterial createdMat = TextureDataHoldingMaterial(currentMaterialName, descriptorLineFields);
                        //materials[currentMaterialName] = createdMat;
                        materials[currentMaterialName] = TextureDataHoldingMaterial(currentMaterialName, descriptorLineFields);
                        // Erase the collector vector for the fields corresponding to a material
                        descriptorLineFields.clear();
                        OMLOGI("Added the following material to the %s library: %s",
                               libFileCstr,
                               currentMaterialName.c_str());
#ifdef DEBUG
                        // In case of debug, we also print out detailed material informations...
                        OMLOGI(" - ka=%d,%d,%d", materials[currentMaterialName].ka[0], materials[currentMaterialName].ka[1], materials[currentMaterialName].ka[2]);
                        OMLOGI(" - kd=%d,%d,%d", materials[currentMaterialName].kd[0], materials[currentMaterialName].kd[1], materials[currentMaterialName].kd[2]);
                        OMLOGI(" - ks=%d,%d,%d", materials[currentMaterialName].ks[0], materials[currentMaterialName].ks[1], materials[currentMaterialName].ks[2]);
                        OMLOGI(" - map_ka=%s", materials[currentMaterialName].map_ka)
                        OMLOGI(" - map_kd=%s", materials[currentMaterialName].map_kd)
                        OMLOGI(" - map_ks=%s", materials[currentMaterialName].map_ks)
                        OMLOGI(" - map_bump=%s", materials[currentMaterialName].map_bump)
#endif
                    }

                    // BEWARE: this changes the line! This is why this is the last call here!
                    currentMaterialName = updateCurrentMaterialName(line);
                } else {
                    // If the line is not a material descriptor, just collect the data
                    // into the string vector for creating the materials later
                    descriptorLineFields.push_back(line);
                }
            }
            // save the last material (we have always saved only in case of newmtl, so we need
            // to do this separately for the last one as there is no newmtl just EOF for that.
            if(descriptorLineFields.size() > 0) {
                // Create a material with all the collected data from the last newmtl entry
                // and add this to the material mapping
                //TextureDataHoldingMaterial createdMat = TextureDataHoldingMaterial(currentMaterialName, descriptorLineFields);
                //materials[currentMaterialName] = createdMat;
                materials[currentMaterialName] = TextureDataHoldingMaterial(currentMaterialName,
                                                                            descriptorLineFields);
                // Erase the collector vector for the fields corresponding to a material
                descriptorLineFields.clear();
                OMLOGI("Added the following material to the %s library: %s",
                       libFileCstr,
                       currentMaterialName.c_str());
#ifdef DEBUG
                // In case of debug, we also print out detailed material informations...
                OMLOGI(" - ka=%d,%d,%d", materials[currentMaterialName].ka[0], materials[currentMaterialName].ka[1], materials[currentMaterialName].ka[2]);
                OMLOGI(" - kd=%d,%d,%d", materials[currentMaterialName].kd[0], materials[currentMaterialName].kd[1], materials[currentMaterialName].kd[2]);
                OMLOGI(" - ks=%d,%d,%d", materials[currentMaterialName].ks[0], materials[currentMaterialName].ks[1], materials[currentMaterialName].ks[2]);
                OMLOGI(" - map_ka=%s", materials[currentMaterialName].map_ka)
                OMLOGI(" - map_kd=%s", materials[currentMaterialName].map_kd)
                OMLOGI(" - map_ks=%s", materials[currentMaterialName].map_ks)
                OMLOGI(" - map_bump=%s", materials[currentMaterialName].map_bump)
#endif
            }
        }

	// Rem.: input is closed with RAII ;-)
    }

    void MtlLib::saveAs(const AssetOutputLibrary &assetOutputLibrary, const char* path, const char* fileName, bool alwaysGrowLibraryFilesList){
	// Add this mtl file to the list if the list is empty or if the user explicitly asks us to always grow the list
	// The latter is useful when we deliberately want to add separate *.mtl files to the single *.obj for some reason...
	// This latter functionality is not in widespread use...
	if(alwaysGrowLibraryFilesList || (libraryFiles.size() == 0)) {
		libraryFiles.push_back(std::string(path) + fileName);
	}
	// open an output stream for us
	OMLOGI("Opening (mtl) output stream for %s%s", path, fileName);
	std::unique_ptr<std::ostream> output = assetOutputLibrary.getAssetOutputStream(path, fileName);

	bool firstMat = true;
	for(auto matNameAndMat : materials){
		std::string matName = matNameAndMat.first;
		// Auto is necessary here for future-proofing so that asText() can be hidden if we ever want to change its implementation!
		auto &mat =  matNameAndMat.second;

		// Put a seperating newline between materials in the resulting *.mtl
		if(firstMat) {
			firstMat = false;
		} else {
			(*output) << std::endl;
		}

		// Print out the lines representing the material in the *.mtl representation
		auto lines = mat.asText();
		for(auto line : lines) {
			output->write(line.c_str(), line.length())<<'\n';
		}
	}
	// Rem.: output is closed with RAII ;-)
    }

    /** Helper function - BEWARE: This changes the argument! */
    std::string MtlLib::updateCurrentMaterialName(char *line) {
        char *savePtr;
        // newmtl
        strtok_r(line, OBJ_DELIMITER, &savePtr);
        // <material_name>
        char *mtlNameCstr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);
        // return a string corresponding the current material name
        return std::string(mtlNameCstr);
    }

    // return the number of materials
    int MtlLib::getMaterialCount() {
        return materials.size();
    }
}
