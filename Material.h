//
// Created by rthier on 2016.04.25..
//

#ifndef NFTSIMPLEPROJ_MATERIAL_H
#define NFTSIMPLEPROJ_MATERIAL_H

#include <string>
#include <vector>
#include <bitset>
#include "objmasterlog.h"

#include <unordered_set> /* - Used for test code */

namespace ObjMaster {
    /**
     * Defines a material. This corresponds to the supported properties of a newmtl in an *.mtl file
     */
    class Material {
    public:
        //
        // General attributes
        //

        /** The name of the material */
        std::string name;

        // TODO: Change the bitset size when necessary...
        /** Defines which fields are used - when creating a material on our own, we must fill this properly! See the various F_* consts for indexing this */
        std::bitset<32> enabledFields;

	// Constants for indexing the bitset
        const static int F_KA = 0;
        const static int F_KD = 1;
        const static int F_KS = 2;
        const static int F_MAP_KA = 3;
        const static int F_MAP_KD = 4;
        const static int F_MAP_KS = 5;
        const static int F_MAP_BUMP = 6;

        //
        // Color attributes
        //

        /** Ambient color */
        std::vector<float> ka;
        /** Diffuse color */
        std::vector<float> kd;
        /** Specular color */
        std::vector<float> ks;

        //
        // Texture assets
        //

        /** ambient texture */
        std::string map_ka;
        /** diffuse texture */
        std::string map_kd;
        /** specular texture */
        std::string map_ks;
        /** bump and map_bump */
        std::string map_bump;

	//
	// Convenience methods for setting fields and enabling them automatically
	//

	// Color attributes convenience methods
	inline void setAndEnableKa(std::vector<float> newKa){
		ka = newKa;
		enabledFields[F_KA] = true;
	}
	inline void setAndEnableKd(std::vector<float> newKd){
		kd = newKd;
		enabledFields[F_KD] = true;
	}
	inline void setAndEnableKs(std::vector<float> newKs){
		ks = newKs;
		enabledFields[F_KS] = true;
	}

	// Texture assets convenience methods
	inline void setAndEnableMapKa(std::string newMapKa){
		map_ka = newMapKa;
		enabledFields[F_MAP_KA] = true;
	}
	inline void setAndEnableMapKd(std::string newMapKd){
		map_kd = newMapKd;
		enabledFields[F_MAP_KD] = true;
	}
	inline void setAndEnableMapKs(std::string newMapKs){
		map_ks = newMapKs;
		enabledFields[F_MAP_KS] = true;
	}
	inline void setAndEnableMapBump(std::string newMapBump){
		map_bump = newMapBump;
		enabledFields[F_MAP_BUMP] = true;
	}

        //
        // Construction
        //

        /**
         * Create a material from a list of fields of lines of the *.mtl - basically this is the parser.
         * This can be used by going through the file and collecting the descriptor lines in a vector
         * until the next material (or the eof) comes.
         *
         * @param name The name of the material
         * @param descriptorLineFields The collected descriptor lines for the material(following newmtl)
         */
        Material(std::string materialName, std::vector<std::string> descriptorLineFields);
	/** Create an empty material with the given name */
        Material(std::string materialName) { name = materialName; }
	/** Create an empty material with no name */
        Material() {};

	/** Clones this material into the other */
	inline void cloneInto(Material *other) {
		other->ka = ka;
		other->kd = kd;
		other->ks = ks;
		other->map_ka = map_ka;
		other->map_kd = map_kd;
		other->map_ks = map_ks;
		other->map_bump = map_bump;
		other->enabledFields = enabledFields;
		other->name = name;
	}

	/** Returns the *.mtl supported text representation as a vector of strings (one per each line) - empty vector is returned for empty material! */
	inline std::vector<std::string> asText() const {
		
		// counter for making unique fallback names
		static int ncnt = 0;

		std::vector<std::string> lines;
		bool hasField = false;

		// always give a name - except if we are not having any field!
		std::string mtlName = name.empty() ? "unknown_" + std::to_string(++ncnt) : name;
		lines.push_back("newmtl " + mtlName);

		// Various color data
		if(enabledFields[Material::F_KA]) {
			lines.push_back("Ka " + std::to_string(ka[0]) + " " + std::to_string(ka[1]) + " " + std::to_string(ka[2]));
			hasField = true;
		}
		if(enabledFields[Material::F_KD]) {
			lines.push_back("Kd " + std::to_string(kd[0]) + " " + std::to_string(kd[1]) + " " + std::to_string(kd[2]));
			hasField = true;
		}
		if(enabledFields[Material::F_KS]) {
			lines.push_back("Ks " + std::to_string(ks[0]) + " " + std::to_string(ks[1]) + " " + std::to_string(ks[2]));
			hasField = true;
		}

		// Various texture data
		if(enabledFields[Material::F_MAP_KA]) {
			lines.push_back("map_Ka " + map_ka);
			hasField = true;
		}
		if(enabledFields[Material::F_MAP_KD]) {
			lines.push_back("map_Kd " + map_kd);
			hasField = true;
		}
		if(enabledFields[Material::F_MAP_KS]) {
			lines.push_back("map_Ks " + map_ks);
			hasField = true;
		}
		if(enabledFields[Material::F_MAP_BUMP]) {
			lines.push_back("map_bump " + map_bump);
			hasField = true;
		}

		// Check for empty material
		if(hasField) {
			// Had a meaningful field - return collected data
			return lines;
		} else {
			// Had no meaningful field - return empty vector!
			return std::vector<std::string>();
		}
	}
    private:
        static std::vector<float> fetchRGBParam(std::string &mtlLine);
        static std::string fetchStringParam(std::string &mtlLine);

        bool static isPrefixOf(std::string a, std::string b);
        bool static isPrefixOf(std::string a, const char* b);
    };


    /** testing output-related operations (like asText()) */
    static int TEST_Material_Output(){
	int errorCount = 0;
	
	// The test-case
	std::vector<std::string> example {
			"newmtl Material_1",
			"Ns 1.960784",
			"Ka 0.000000 0.000000 0.000000",
			"Kd 0.301176 0.301176 0.301176",
			"Ks 0.045000 0.045000 0.045000",
			"Ni 1.000000",
			"d 1.000000",
			"illum 2",
			"map_Kd witch_hat(color).jpg",
			"map_Bump witch_hat(normal).jpg",
			"map_Ka witch_hat(color).jpg",
			"map_Ks witch_hat(specular).jpg",
	};

	// The expected result after parsing and outputting a test case
	std::unordered_set<std::string> expectedLines {
		"newmtl Material_1",
		"Ka 0.000000 0.000000 0.000000",
		"Kd 0.301176 0.301176 0.301176",
		"Ks 0.045000 0.045000 0.045000",
		"map_Ka witch_hat(color).jpg",
		"map_Kd witch_hat(color).jpg",
		"map_Ks witch_hat(specular).jpg",
		"map_bump witch_hat(normal).jpg",
	};

	// Parse
	Material m1("Material_1", example);

	// Output
	std::vector<std::string> output = m1.asText();

	// Test1 (parsed)
#ifdef DEBUG
	OMLOGI("Parsed mtl material asText returns:");
	OMLOGI("-----------------------------------");
#endif
	for(auto s : output){
#ifdef DEBUG
		OMLOGI("%s", s.c_str());
#endif
		auto got = expectedLines.find(s);
		if(got == expectedLines.end()){
			// Got something unexpected
			OMLOGE("Unexpected mtl line in asText() output: %s", s.c_str());
			OMLOGI("This asText error might mean that the tests or the code is broken!");
			++errorCount;	// Indicate error
		}
	}

	// The expected result for second test case
	std::unordered_set<std::string> expectedLines2 {
		"newmtl Material_2",
		"Kd 1.000000 0.200000 0.200000",
		"map_Kd my_diff_tex.png",
		"map_bump my_bump_tex.png",
	};

	// Create new
	Material m2("Material_2");
	// with reddish diffuse color and some simple textures
	m2.setAndEnableKd({1.0f, 0.2f, 0.2f});
	m2.setAndEnableMapKd("my_diff_tex.png");
	m2.setAndEnableMapBump("my_bump_tex.png");

	// Output new
	output = m2.asText();

	// Test2 (new)
#ifdef DEBUG
	OMLOGI("Parsed mtl material asText returns:");
	OMLOGI("-----------------------------------");
#endif
	for(auto s : output){
#ifdef DEBUG
		OMLOGI("%s", s.c_str());
#endif
		auto got = expectedLines2.find(s);
		if(got == expectedLines2.end()){
			// Got something unexpected
			OMLOGE("Unexpected mtl line in asText() output: %s", s.c_str());
			OMLOGI("This asText error might mean that the tests or the code is broken!");
			++errorCount;	// Indicate error
		}
	}

	return errorCount;
    }

    /** Test if parsing and loading stuff works */
    static bool TEST_Material() {
#ifdef DEBUG
        OMLOGI("TEST_Material...");
#endif

        Material test1 = Material("test1", std::vector<std::string> {
                std::string("Ka 1.0 2.0 4.0"),
                std::string("badline"),
                std::string("map_Ka ambient.png"),
                std::string("badline")
        });

        // Test "enabledFields"
        if(!test1.enabledFields[Material::F_MAP_KA]) {
            OMLOGE("TEST_Material: enabledFields[F_MAP_KA] error!");
            return false;
        }
        if(!test1.enabledFields[Material::F_KA]) {
            OMLOGE("TEST_Material: enabledFields[F_KA] error!");
            return false;
        }
        if(test1.enabledFields[Material::F_KD]) {
            OMLOGE("TEST_Material: enabledFields[F_KD] error!");
            return false;
        }
        if(test1.enabledFields[Material::F_KS]) {
            OMLOGE("TEST_Material: enabledFields[F_KS] error!");
            return false;
        }
        if(test1.enabledFields[Material::F_MAP_KD]) {
            OMLOGE("TEST_Material: enabledFields[F_MAP_KD] error!");
            return false;
        }
        if(test1.enabledFields[Material::F_MAP_KS]) {
            OMLOGE("TEST_Material: enabledFields[F_MAP_KS] error!");
            return false;
        }
        if(test1.enabledFields[Material::F_MAP_BUMP]) {
            OMLOGE("TEST_Material: enabledFields[F_MAP_BUMP] error!");
            return false;
        }

        // Test values
        if(test1.ka[0] != 1.0f) {
            OMLOGE("TEST_Material: Ka error 1");
            return false;
        }
        if(test1.map_ka != "ambient.png") {
            OMLOGE("TEST_Material: map_Ka error 1: %s", test1.map_ka.c_str());
            return false;
        }

#ifdef DEBUG
        OMLOGI("...TEST_Material success!");
#endif
        // All checks hopefully passed!
        return true;
    }
}

#endif //NFTSIMPLEPROJ_MATERIAL_H
