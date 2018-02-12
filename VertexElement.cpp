//
// Created by rthier on 2016.04.08..
//

//#include "objmasterlog.h"
#include "VertexElement.h"
#include <cstring>    /* strtok_r, strdup */
#include <cstdlib>     /* atof */
#include "ObjCommon.h"
#include "wincompat.h" // msvc hax

namespace ObjMaster {
    bool VertexElement::isParsable(const char *fields) {
        // Not an empty string, the first character is a v and there is no second char in the key...
        return (fields != nullptr) && (fields[0] != 0) && (fields[0] == 'v') && (fields[1] == ' ');
    }

    VertexElement::VertexElement(const char *fields) {
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
    VertexElement::VertexElement(char *fields) {
        constructionHelper(fields);
    }

    void VertexElement::constructionHelper(char *fields) {
        if(isParsable(fields)) {
            char *savePtr;
            char *key = strtok_r(fields, OBJ_DELIMITER, &savePtr);
            char *xStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);
            char *yStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);
            char *zStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);

            x = (float)atof(xStr);
            y = (float)atof(yStr);
            z = (float)atof(zStr);
        }
    }
}
