//
// Created by rthier on 2016.04.11..
//



#include "FaceElement.h"
#include <cstring>    /* strtok_r */
#include "Obj.h"
#include "wincompat.h" // msvc hax

namespace ObjMaster {
    bool FaceElement::isParsable(const char *fields) {
        // Not an empty string, the first character is an 'f' the second is a ' '
        return (fields != nullptr) && (fields[0] != 0) && (fields[0] == 'f') && (fields[1] == ' ');
    }

    FaceElement::FaceElement(const char *fields) {
        // The strtok_r changes the string so we need to duplicate it
        char *copy = strdup(fields);
        constructionHelper(copy);
        free(copy);
    }

    // This variant modifies the provided 'string' but it is faster as it do this without the copy
    FaceElement::FaceElement(char *fields) {
        constructionHelper(fields);
    }

    // Common parts of constructors
    void FaceElement::constructionHelper(char* fields) {
        if (isParsable(fields)) {
            // Count delimiters
            // The first delimiter delimits the 'f' and the first FacePoint
            // so to get the number of facePoints all we need to do is to count the delimiters!
            int delimCount = 0;
            for (int i = 0; (fields[i] != 0); ++i) {
                if (fields[i] == OBJ_DELIMITER[0]) {
                    ++delimCount;
                }
            }

            // Tokenization
            char *savePtr;
            // The 'f' is the first token...
            char *key = strtok_r(fields, OBJ_DELIMITER, &savePtr);

            // This both sets the facePointCount and fills in the face-points
            // The facePointCount can be less than the delimCount when we handle only 3 points and
            // there are more delimiters than that!
            for (facePointCount = 0;
                 facePointCount < delimCount && facePointCount < MAX_FACEPOINT_COUNT;
                 ++facePointCount) {

                // Extract the currently latest FacePoint
                char *facePointStr = strtok_r(nullptr, OBJ_DELIMITER, &savePtr);
                // Parse the given facePoint and save it
                facePoints[facePointCount] = FacePoint(facePointStr, true);
            }
        }
    }
}
