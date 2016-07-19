//
// Created by rthier on 2016.04.08..
//

//#include "objmasterlog.h"
#include "VertexElement.h"
#include <cstring>    /* strtok_r, strdup */
#include <cstdlib>     /* atof */
#include "Obj.h"

namespace ObjMaster {
    bool VertexElement::isParsable(const char *fields) {
        // Not an empty string, the first character is a v and there is no second char in the key...
        return (fields != nullptr) && (fields[0] != 0) && (fields[0] == 'v') && (fields[1] == ' ');
    }

    VertexElement::VertexElement(const char *fields) {
        // The strtok_r changes the string so we need to duplicate it
        char *copy = strdup(fields);
        constructionHelper(copy);
        free(copy);
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

            x = atof(xStr);
            y = atof(yStr);
            z = atof(zStr);
        }
    }
}