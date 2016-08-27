//
// Created by rthier on 2016.04.11..
//

//#include "objmasterlog.h"
#include "VertexNormalElement.h"
#include <cstring>    /* strtok_r */
#include <cstdlib>     /* atof */
#include "Obj.h"
#include "wincompat.h" // msvc hax

namespace ObjMaster {
    bool VertexNormalElement::isParsable(const char *fields) {
        // Not an empty string, the first character is a 'v' the second is an 'n' and then a space
        return (fields != nullptr) && (fields[0] != 0) && (fields[0] == 'v') && (fields[1] == 'n')
                && fields[2] == ' ';
    }

    VertexNormalElement::VertexNormalElement(const char *fields) {
        // The strtok_r changes the string so we need to duplicate it
        char *copy = strdup(fields);
        constructionHelper(copy);
        free(copy);
    }

// This variant modifies the provided 'string' but is it faster as it do this without the copy
    VertexNormalElement::VertexNormalElement(char *fields) {
        constructionHelper(fields);
    }

    void VertexNormalElement::constructionHelper(char *fields) {
        if (isParsable(fields)) {
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
