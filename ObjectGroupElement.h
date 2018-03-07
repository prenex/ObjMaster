//
// Created by rthier on 2016.06.21..
//

#ifndef NFTSIMPLEPROJ_OBJECTGROUPELEMENT_H
#define NFTSIMPLEPROJ_OBJECTGROUPELEMENT_H

#include <string>
#include "objmasterlog.h"

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

	/** Gives the objmaster-used standard obj-representation given the group name. */
	inline static std::string asText(std::string groupName) {
		// Use 'o' in case of default
		return asTextO(groupName);
	}

	/** Gives the obj-representation given the group name. Uses 'o' */
	inline static std::string asTextO(std::string groupName) {
		return "o " + groupName;
	}

	/** Gives the obj-representation given the group name. Uses 'g' */
	inline static std::string asTextG(std::string groupName) {
		return "g " + groupName;
	}
    };
    /** testing output-related operations (like asText()) */
    static int TEST_ObjectGroupElement_Output(){
		const char *test = "o testGroupName";
		// Parse
		std::string groupName = ObjectGroupElement::getObjectGroupName(test);
		// Reparse result into *.obj descriptor
		auto stro1 = ObjectGroupElement::asText(groupName);
		auto stro2 = ObjectGroupElement::asTextO(groupName);
		auto strg = ObjectGroupElement::asTextG(groupName);
		// Compare original and reparsed - this should test output reasonably well
		if((stro1 == std::string(test)) && (stro2 == stro1) && (strg == "g testGroupName")) {
			// OK
			return 0;
		} else {
			// ERROR
			OMLOGE("Bad ObjectGroupElement output: (%s); (%s); (%s) instead of %s", stro1.c_str(), stro2.c_str(), strg.c_str(), test);
			return 1;
		}
    }
}


#endif //NFTSIMPLEPROJ_OBJECTGROUPELEMENT_H
