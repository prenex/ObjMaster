//
// Created by rthier on 2016.04.11..
//

//#include "objmasterlog.h"
#include "VertexTextureElement.h"
#include <cstring>    /* strtok_r */
#include <cstdlib>     /* atof */
#include "ObjCommon.h"
#include "wincompat.h" // msvc hax

namespace ObjMaster {
    bool VertexTextureElement::isParsable(const char *fields) {
        // Not an empty string, the first character is a 'v' the second is an 't' and then a space
        return (fields != nullptr) && (fields[0] != 0) && (fields[0] == 'v') && (fields[1] == 't')
               && fields[2] == ' ';
    }

    VertexTextureElement::VertexTextureElement(const char *fields) {
        // The strtok_r changes the string so we need to duplicate it
        char copy[256];	// The line never should be longer than this anyways...
	copy[255] = 0;	// ensure terminator
        strncpy(copy, fields, 254);
        constructionHelper(copy);
	// Remark: The original code below makes very small malloc and free pairs and fragment memory!
        // char *copy = strdup(fields);
        // constructionHelper(copy);
        // free(copy);
    }

    // This variant modifies the provided 'string' but is it faster as it do this without the copy
    VertexTextureElement::VertexTextureElement(char *fields) {
        constructionHelper(fields);
    }

    void VertexTextureElement::constructionHelper(char *fields) {
        if (isParsable(fields)) {
            char *savePtr;
            char *key = strtok_r(fields, OBJ_DELIMITER, &savePtr);
            char *uStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);
            char *vStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);

            u = (float)atof(uStr);
            v = (float)atof(vStr);
        }
    }
}
