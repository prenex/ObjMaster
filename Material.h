//
// Created by rthier on 2016.04.25..
//

#ifndef NFTSIMPLEPROJ_MATERIAL_H
#define NFTSIMPLEPROJ_MATERIAL_H

#include <string>
#include <vector>
#include <bitset>
#include "objmasterlog.h"

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

        /** Defines which fields can be used. See the various F_* consts for indexing this */
        // TODO: Change the bitset size when necessary...
        std::bitset <32> enabledFields;

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
        std::vector <float> ka;
        /** Diffuse color */
        std::vector <float> kd;
        /** Specular color */
        std::vector <float> ks;

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
        Material(std::string materialName, std::vector <std::string> descriptorLineFields);
        Material() {};

    private:
        static std::vector<float> fetchRGBParam(std::string &mtlLine);
        static std::string fetchStringParam(std::string &mtlLine);

        bool static isPrefixOf(std::string a, std::string b);
        bool static isPrefixOf(std::string a, const char* b);
    };


    /** Test if parsing and stuff works */
    static bool TEST_Material() {
#ifdef DEBUG
        OMLOGE("TEST_Material...");
#endif

        Material test1 = Material::Material("test1", (std::vector<std::string>) {
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
        OMLOGE("...TEST_Material success!");
#endif
        // All checks hopefully passed!
        return true;
    }
}

#endif //NFTSIMPLEPROJ_MATERIAL_H
