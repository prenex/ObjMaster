//
// Created by rthier
//

#include "LineElement.h"
#include <cstring>    /* strtok_r, strdup */
#include <cstdlib>     /* atof */
#include "wincompat.h" /* msvc hax */
#include "ObjCommon.h" /* OBJ_DELIMITER */

namespace ObjMaster {
    bool LineElement::isParsable(const char *fields) {
        // Not an empty string, the first character is a 'l' and there is no second char in the key...
        return (fields != nullptr) && (fields[0] != 0) && (fields[0] == 'l') && (fields[1] == ' ');
    }

    LineElement::LineElement(const char *fields) {
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
    LineElement::LineElement(char *fields) {
        constructionHelper(fields);
    }

    void LineElement::constructionHelper(char *fields) {
        if(isParsable(fields)) {
            char *savePtr;
            char *key = strtok_r(fields, OBJ_DELIMITER, &savePtr);
            char *bStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);
            char *eStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);

            bVindex = (int)atoi(bStr);
            eVindex = (int)atoi(eStr);
        }
    }
}

// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
