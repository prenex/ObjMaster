#ifndef OBJMASTER_LINELEMENT_H
#define OBJMASTER_LINELEMENT_H

#include <string>

#include "objmasterlog.h"

namespace ObjMaster {

	/** The 'l' element in obj files */
	class LineElement final {
	public:
		/** The begin vertex */
		int bVindex;
		/** The end vertex */
		int eVindex;

		// Copies are defeaulted
		LineElement(const LineElement &other) = default;
		LineElement& operator=(const LineElement &other) = default;
		// Moves are defaulted
		LineElement(LineElement &&other) = default;
		LineElement& operator=(LineElement &&other) = default;

		/** Create an emtpy line element */
		LineElement() {};

		/** Crete a line element l <beginVindex> <endVindex> */
		LineElement(int beginVindex, int endVindex) { bVindex = beginVindex; eVindex = endVindex; };

		/** Parse a line element */
		LineElement(char *fields);

		/** Parse a line element */
		LineElement(const char *fields);

		/** Quick check that tells if an obj file line is parsable as a line element or not */
		static bool isParsable(const char *fields);

		/** Gets the textual representation */
		inline std::string asText() {
			return "l " + std::to_string(bVindex) + " " + std::to_string(eVindex);
		}
	private:
		void constructionHelper(char *fields);
	};
    /** testing output-related operations (like asText()) */
    static int TEST_LineElement_Output(){
		const char *test = "l 1 2";
		// Parse
		LineElement l(test);
		// Get as string
		auto str = l.asText();
		// Reparse result
		LineElement l2(str.c_str());
		// Compare original and reparsed - this should test output reasonably well
		if((l.bVindex == l2.bVindex) && (l.eVindex == l2.eVindex)) {
			// OK
			return 0;
		} else {
			// ERROR
			OMLOGE("Bad LineElement output: %s instead of %s", str.c_str(), test);
			return 1;
		}
    }
}

#endif // OBJMASTER_LINELEMENT_H
// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
