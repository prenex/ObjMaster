//
// Created by rthier on 2016.07.15..
//

#ifndef NFTSIMPLEPROJ_USEMTL_H
#define NFTSIMPLEPROJ_USEMTL_H

#include <string>
#include "funhelper.h"
#include <cstring>    /* strtok_r, strdup */
#include "Obj.h"
#include "wincompat.h" // msvc hax

namespace ObjMaster {
    class UseMtl {
    public:
        /** Defines if the given fields of an obj file line can be parsed as a UseMtl or not */
        static bool isParsable(const char *fields) {
            if((fields != nullptr) && fields[0] == 'u' && isStartsWith(std::string(fields), "usemtl")) {
                return true;
            } else {
                return false;
            }
        }

        /**
         * Fetches the name of the material the fields are describing.
         * should be used after isParsable(..) or in case we are sure that this can run!
         */
        static std::string fetchMtlName(const char *fields) {
            char *savePtr;
            // usemtl
            char* copy = strdup(fields);
            strtok_r(copy, OBJ_DELIMITER, &savePtr);
            // <material_name>
            char *mtlNameCstr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);
            // return a string corresponding the current material name
			std::string ret = std::string(mtlNameCstr);
			free(copy);
	    return ret;
        }

	/** Gets the textual representation for *.obj generation */
	inline std::string asText(std::string materialName) {
		return "usemtl " + materialName;
	}

    };
}

#endif //NFTSIMPLEPROJ_USEMTL_H
