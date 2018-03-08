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

#include <map> /* for saveAs *.obj compacting with ordered operations */

#define MAT_SEP ":mtl:"
//#define DEBUG 1

namespace ObjMaster {
    Obj::Obj(const AssetLibrary &assetLibrary, const char *path, const char *fileName) {
        constructionHelper(assetLibrary, path, fileName, Obj::EXPECTED_VERTEX_DATA_NUM, Obj::EXPECTED_FACES_NUM);
    }
    Obj::Obj(const AssetLibrary &assetLibrary, const char *path, const char *fileName, int expectedVertexDataNum, int expectedFaceNum) {
        constructionHelper(assetLibrary, path, fileName, expectedVertexDataNum, expectedFaceNum);
    }
    /** Save this MtlLib as a *.mtl - using the path, fileName and the provided asset-out library */
    void Obj::saveAs(const AssetOutputLibrary &assetOutputLibrary, const char* path, const char* fileName, bool saveAsMtlToo, ObjSaveModeFlags saveMode){
	OMLOGI("Opening (obj) output stream for %s%s", path, fileName);
	std::unique_ptr<std::ostream> output = assetOutputLibrary.getAssetOutputStream(path, fileName);

	// saveAs our mtlLib if the caller wants that too and there is anything to write
	// Also the mtl lib is not saved if the saveMode does not use materials at all
	if(saveAsMtlToo && (((int)saveMode & ObjSaveModeFlags::MATERIALS_GEOMETRY) != 0)) {
		// Generate proper *.mtl file name using the provided *.obj file name
		// If there is .*obj extension, we change that to *.mtl if there were none, we just add...
		std::string fn(fileName);
		size_t lastIndex = fn.find_last_of("."); 
		std::string rawName;
		if(lastIndex == std::string::npos) {
			// There were no *.obj extension or any extension
			rawName = fn;
		} else {
			// There were an extension - remove that
			rawName = fn.substr(0, lastIndex); 
		}
		// And *.mtl ending for getting the extension
		std::string mtlFileName = rawName + ".mtl";

		// Save the *.mtl file near the *.obj
		mtlLib.saveAs(assetOutputLibrary, path, mtlFileName.c_str());
	}

	// Reference the (already existing or generated) *.mtl file(s) - if save mode saves materials
	if(((int)saveMode & ObjSaveModeFlags::MATERIALS_GEOMETRY) != 0) {
		auto mtlLibLine = mtlLib.asText();
		output->write(mtlLibLine.c_str(), mtlLibLine.length())<<'\n';
	}

	// Write out all vertex data as 'v's
	for(auto v : vs) {
		auto line = v.asText();
		output->write(line.c_str(), line.length())<<'\n';
	}
	// Write out all vertex data as 'vt's
	for(auto vt : vts) {
		auto line = vt.asText();
		output->write(line.c_str(), line.length())<<'\n';
	}
	// Write out all vertex data as 'vn's
	for(auto vn : vns) {
		auto line = vn.asText();
		output->write(line.c_str(), line.length())<<'\n';
	}


	// Write out faces, groups, materials
	// ----------------------------------

	if((int)saveMode == ObjSaveModeFlags::ONLY_UNGROUPED_GEOMETRY) {
		// Simples case: only geometry - just write out faces
		for(auto f : fs) {
			auto line = f.asText();
			output->write(line.c_str(), line.length())<<'\n';
		}
	} else {
		// For proper compact output generation we need the objMatFaceGroups sorted
		std::map<std::string, ObjectMaterialFaceGroup> sortedObjectMaterialGroups;
		bool gbit = false;

		// The sorting key should be either matName:err:groupName or groupName:mtl:matName according to mode!
		// This ensures that those face elements we can compact together are near each other!
		for(auto kv : objectMaterialGroups) {
			if(((int)saveMode == ObjSaveModeFlags::GROUPS_GEOMETRY) ||
			   ((int)saveMode == ObjSaveModeFlags::MATERIALS_AND_GROUPS)) {
				// set the gbit here
				if(((int)saveMode & ObjSaveModeFlags::G) != 0){
					gbit = true; // indicate 'g' usage
				}

				// groupName:mtl:matName - it is already available
				// Rem.: Both above cases we need to sort by groups basically!
				std::string key = kv.first;
				sortedObjectMaterialGroups[key] = kv.second;
			} else if((int)saveMode == ObjSaveModeFlags::MATERIALS_GEOMETRY) {
				// matName:err:groupName
				// Rem.: We use :err: deliberately to force ourself handling things as it should!
				//       We cannot just save out these keys when generating real output - only used for sorting!
				std::string key = kv.second.textureDataHoldingMaterial.name + ":err:" + kv.second.objectGroupName;
				sortedObjectMaterialGroups[key] = kv.second;
			} else {
				// This should never happen - unless someone breaks ObjMaster!
				OMLOGE("Code is utterly broken! Unkown savemode!");
				exit(0);
			}
		}

		// Print out stuff
		std::string currentMat;	// material now used
		std::string currentGrp;	// group now used
		for(auto skv : sortedObjectMaterialGroups) {
			std::string &matName = skv.second.textureDataHoldingMaterial.name;
			std::string &grpName = skv.second.objectGroupName;

			// See if we need to print out groups or not - and if we need: then see if group has changed or not!
			if((((int)saveMode & ObjSaveModeFlags::GROUPS_GEOMETRY) != 0) && (currentGrp != grpName)) {
				// Write out the group
				std::string line = ObjectGroupElement::asTextO(grpName);
				if(gbit) {
					line = ObjectGroupElement::asTextG(grpName);
				}
				output->write(line.c_str(), line.length())<<'\n';

				// Erase current material (as we better always restart when the groups have changed!)
				currentMat = "";

				// Update current group
				currentGrp = grpName;
			}
		}
	}
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
