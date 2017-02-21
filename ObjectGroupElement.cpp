//
// Created by rthier on 2016.06.21..
//

#include "ObjectGroupElement.h"

namespace ObjMaster {
    /** Decides if the given fields can be parsed as an ObjectGroupElement */
    bool ObjectGroupElement::isParsable(const char *fields){
        // Not an empty string, the first character is an o and there is no second char of the key!
        return (fields != nullptr) && (fields[0] != 0) && (fields[0] == 'o') && (fields[1] == ' ');
    }

    /** Given the fields that describe an ObjectGroupElement, return the name part */
    std::string ObjectGroupElement::getObjectGroupName(const char *fields){
        // Just return a string that is initialized with the remaining part of the fields/line
        return std::string{fields+2};
    }
}
