//
// Created by rthier on 2016.06.21..
//

#ifndef NFTSIMPLEPROJ_OBJECTGROUPELEMENT_H
#define NFTSIMPLEPROJ_OBJECTGROUPELEMENT_H

#include <string>

namespace ObjMaster {
    /**
     * Helper class for handling "o" object groups of the obj files
     */
    class ObjectGroupElement final {
    public:
        /** Decides if the given fields can be parsed as an ObjectGroupElement */
        static bool isParsable(const char *fields);

        /** Given the fields that describe an ObjectGroupElement, return the name part */
        static std::string getObjectGroupName(const char *fields);
    };
}


#endif //NFTSIMPLEPROJ_OBJECTGROUPELEMENT_H
