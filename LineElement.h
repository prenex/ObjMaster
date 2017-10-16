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
}

#endif // OBJMASTER_LINELEMENT_H
// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
