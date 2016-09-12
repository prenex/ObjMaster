//
// Created by rthier on 2016.04.25..
//

#define DEBUG

#include "Material.h"
#include "Obj.h"        /* OBJ_DELIMITER */
#include <cstring>      /* strtok_r, strdup */
#include "objmasterlog.h"
#include "wincompat.h" // msvc hax

namespace ObjMaster {

    // Helper functions to see if a string is a prefix of an other or not...
    bool Material::isPrefixOf(std::string a, const char* b){
        std::string bStr = std::string(b);
        return isPrefixOf(a, bStr);
    }

    bool Material::isPrefixOf(std::string a, std::string b){
        if (a.compare(0, b.length(), b) == 0){
            return true;
        } else {
            return false;
        }
    }

    // Private helper method to fetch rgb values
    std::vector<float> Material::fetchRGBParam(std::string &mtlLine) {
        // Tokenize the string
        char *copy = strdup(mtlLine.c_str());
        char *savePtr;
        char *key = strtok_r(copy, OBJ_DELIMITER, &savePtr);
        char *rStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);
        char *gStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);
        char *bStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);

        // Convert to float values
        float r = (float)atof(rStr);
        float g = (float)atof(gStr);
        float b = (float)atof(bStr);

		// This should be after atofs as the tokenizer refers to memory in the copy!
        free(copy); // strtok modifies the string so we copied it above...

        // Return created vector
        return std::vector<float> {r, g, b};
    }

    // Private helper method to fetch a string parameter
    std::string Material::fetchStringParam(std::string &mtlLine) {
        // Find delimiter
        std::size_t pos = mtlLine.find(OBJ_DELIMITER);
        // Return part of string after the first delimiter
        return mtlLine.substr(pos + 1);
    }

    Material::Material(const std::string materialName,
                       std::vector <std::string> descriptorLineFields) : name{materialName} {
        // Temporal variables
        std::vector <float> ka, kd, ks;
        std::string map_ka, map_kd, map_ks, map_bump;

        // Try parsing the descriptor lines collected for the material
        for (auto &mtlLine : descriptorLineFields) {
#ifdef DEBUG
OMLOGD("Parsing line: %s", mtlLine.c_str());
#endif
            // Color descriptors
            if (mtlLine.length() > 1 && mtlLine[0] == 'K') {
#ifdef DEBUG
OMLOGD("Color descriptor candidate line: %s", mtlLine.c_str());
#endif
                if (mtlLine[1] == 'a') {
                    // Ka
                    ka = fetchRGBParam(mtlLine);
                    enabledFields[F_KA] = true;
                    continue;
                }
                if (mtlLine[1] == 'd') {
                    // Kd
                    kd = fetchRGBParam(mtlLine);
                    enabledFields[F_KD] = true;
                    continue;
                }
                if (mtlLine[1] == 's') {
                    // Ks
                    ks = fetchRGBParam(mtlLine);
                    enabledFields[F_KS] = true;
                    continue;
                }
            }

            // Texture map descriptors
            // First do a fast check on the first letter!
            if (mtlLine.length() > 0 && (mtlLine[0] == 'm' || mtlLine[0] == 'b')) {
#ifdef DEBUG
OMLOGD("Texture descriptor candidate line: %s", mtlLine.c_str());
#endif
                if (mtlLine.compare(0, std::string("map_Ka").length(), std::string("map_Ka")) == 0) {
                    // map_Ka
                    map_ka = fetchStringParam(mtlLine);
                    enabledFields[F_MAP_KA] = true;
                    continue;
                }
                if (mtlLine.compare(0, std::string("map_Kd").length(), std::string("map_Kd")) == 0) {
                    // map_Kd
                    map_kd = fetchStringParam(mtlLine);
                    enabledFields[F_MAP_KD] = true;
                    continue;
                }
                if (mtlLine.compare(0, std::string("map_Ks").length(), std::string("map_Ks")) == 0) {
                    // map_Ks
                    map_ks = fetchStringParam(mtlLine);
                    enabledFields[F_MAP_KS] = true;
                    continue;
                }
                if (mtlLine.compare(0, std::string("map_bump").length(), std::string("map_bump")) == 0) {
                    // map_bump
                    map_bump = fetchStringParam(mtlLine);
                    enabledFields[F_MAP_BUMP] = true;
                    continue;
                }
            }
        }

        // Save the found data
        this->ka = ka;
        this->kd = kd;
        this->ks = ks;
        this->map_ka = map_ka;
        this->map_kd = map_kd;
        this->map_ks = map_ks;
        this->map_bump = map_bump;
    }
}

