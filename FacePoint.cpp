//
// Created by rthier on 2016.04.11..
//

#include "FacePoint.h"
#include <cstring>    /* strtok_r */
#include <cstdlib>     /* atoi */
#include "wincompat.h" // msvc hax

namespace ObjMaster {
    bool FacePoint::isParsable(const char *fields) {
        // Basic check (for speed): Not an empty string and the first character is a number...
        if((fields != nullptr) && (fields[0] != 0) && (fields[0] >= '0') && (fields[0] <= '9')) {
            // TODO: still not a full-blown check, but at least do most of the things...
            int delimCount = 0;
            for(int i = 0; (fields[i] != 0) && (i < FACEPOINT_MAXLEN); ++i) {
                if(fields[i] == FACEPOINT_DELIMITER[0]) {
                    ++delimCount;
                }
            }
            if(delimCount == 2) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }


    // No-arg construction
    FacePoint::FacePoint() : vIndex(0), vtIndex(0), vnIndex(0) { }

    // Variant that keeps the original string as it is. Has overhead because of copies.
    FacePoint::FacePoint(const char *fields) {
        if (isParsable(fields)) {
		// The strtok_r changes the string so we need to duplicate it
		char copy[256];	// The line never should be longer than this anyways...
		copy[255] = 0;	// ensure terminator
		strncpy(copy, fields, 254);
		constructorHelper(copy);
		// Remark: The original code below makes very small malloc and free pairs and fragment memory!
		// char *copy = strdup(fields);
		// constructionHelper(copy);
		// free(copy);
        }
    }

    // This variant modifies the provided 'string' but is it faster as it do this without the copy
    FacePoint::FacePoint(char *fields) {
        if (isParsable(fields)) {
            constructorHelper(fields);
        }
    }

    // Constructor variant where you can decide if you want to avoid unnecessary "isParsable"
    // Useful when you are sure about the data or did call the check earlier manually
    // so that this way you can avoid unnecessary duplicated checks
    FacePoint::FacePoint(const char *fields, bool forceParser) {
        if (forceParser || isParsable(fields)) {
		// The strtok_r changes the string so we need to duplicate it
		char copy[256];	// The line never should be longer than this anyways...
		copy[255] = 0;	// ensure terminator
		strncpy(copy, fields, 254);
		constructorHelper(copy);
		// Remark: The original code below makes very small malloc and free pairs and fragment memory!
		// char *copy = strdup(fields);
		// constructionHelper(copy);
		// free(copy);
        }
    }

    // Constructor variant where you can decide if you want to avoid unnecessary "isParsable"
    // Useful when you are sure about the data or did call the check earlier manually
    // so that this way you can avoid unnecessary duplicated checks
    FacePoint::FacePoint(char *fields, bool forceParser) {
        if (forceParser || isParsable(fields)) {
            constructorHelper(fields);
        }
    }

    // Common parts of the constructors
    void FacePoint::constructorHelper(char* fields) {
	// Fill vIndex, vtIndex, vnIndex values by parsing
        char *savePtr;
	// We need to trick a bit because strtok_r skips continous delimiters!
	// Bacause of this we first load the values into these auxiliary variables and use a simple rule set to handle skipping
	unsigned int a, b, c;
        // Fill-in the indices - the nullptr checks are necessary otherwise we get an abort for segfaulting!
        char* vIndexStr = strtok_r(fields, FACEPOINT_DELIMITER, &savePtr);
	if (vIndexStr != nullptr) {
		a = atoi(vIndexStr) - 1;
	} else {
		a = -1;	// Indicates missing data
	}
	if(a != -1) {
		char* vtIndexStr = strtok_r(nullptr, FACEPOINT_DELIMITER, &savePtr);
		if (vtIndexStr != nullptr) {
			b = atoi(vtIndexStr) - 1;
		} else {
			b = -1;	// Indicates missing data
		}
		if(b != -1) {
			char* vnIndexStr = strtok_r(nullptr, FACEPOINT_DELIMITER, &savePtr);
			if (vnIndexStr != nullptr) {
				c = atoi(vnIndexStr) - 1;
			} else {
				c = -1;	// Indicates missing data
			}
			if(c != -1) {
				// Best case: all values have been provided
				// and these values are v, vt, vn. Let this be the fastest and default - as we have handled it always!
				vIndex = a;
				vtIndex = b;
				vnIndex = c;
				return;
			} else {
				// One value was missing - but because of strtok_r we don't know which is it!
				if(fields != vIndexStr) {
					// If the first part doesn't start where the original string starts,
					// that is the missing one, because the tokenizer went over the delimiter
					vIndex = -1;
					// Also we need to handle the offset so vt is the first parsed and vn is second!
					vtIndex = a;
					vnIndex = b;
				} else {
					// vIndex is not missing and is in 'a' if the above was false
					vIndex = a;
					// and we can be sure that there is an earlier character for vtIndexStr pointer
					// where we can see if that is a delimiter or not. See man strtok_r why this works!
					if(*(vtIndexStr-1) == FACEPOINT_DELIMITER[0]) {
						// If there is a delimiter before the second existing thing
						// that means the second was missing and 'b' contains the third data
						// This is very probable too
						vtIndex = -1;
						vnIndex = b;
						return;
					} else {
						// If we come here the normals were missing (no other chance - see above)
						vtIndex = b;
						vnIndex = -1;
						return;
					}
				}
			}
		} else {
			// Two values were missing - This is the same as knowing there is only one value
			//  but because of strtok_r we don't know which one exists!
			if(fields == vIndexStr) {
				// If the first pointer did not moved that is the existing one
				// This is actually probable in many cases so it became a fast path
				vIndex = a;
				vtIndex = -1;
				vnIndex = -1;
				return;
			} else {
				if(fields == (vIndexStr - 1)) {
					// If the one existing pointer is only one further from beginning
					// that means only the first value was skipped so the second exists
					// This is not probable and is a really weird combination...
					vIndex = -1;
					vtIndex = a;
					vnIndex = -1;
					return;
				} else {
					// Otherwise there must be two delimiter that has been skipped
					// thus the one value is for the last
					// This is not probable at all - even more weird maybe...
					vIndex = -1;
					vtIndex = -1;
					vnIndex = a;
					return;
				}
			}
		}
	} else {
		// All three values are missing! Should never happen!
		OMLOGE("Completely empty FacePoint found! Try to completely ignore!");
		vIndex = -1;
		vtIndex = -1;
		vnIndex = -1;
	}
    }
}
