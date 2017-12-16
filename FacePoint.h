//
// Created by rthier on 2016.04.11..
//

#ifndef NFTSIMPLEPROJ_FACEPOINT_H
#define NFTSIMPLEPROJ_FACEPOINT_H

#include "objmasterlog.h"

namespace ObjMaster {

    static const char *FACEPOINT_DELIMITER = "/";
    static const int FACEPOINT_MAXLEN = 1024;

    class FacePoint {
    public:

        unsigned int vIndex;
        unsigned int vtIndex;
        unsigned int vnIndex;

        FacePoint();
        FacePoint(char *fields);
        FacePoint(const char *fields);
        FacePoint(char *fields, bool forceParser);
        FacePoint(const char *fields, bool forceParser);
        static bool isParsable(const char *fields);

	/** Gets the textual representation */
	inline std::string asText() {
		return std::to_string(vIndex) + "/" + std::to_string(vtIndex) + "/" + std::to_string(vnIndex);
	}

    private:
        // Common parts of constructors
        void constructorHelper(char *fields);
    };

// Very simple unit-testing approach
// it is better to have these run on the device itself and callable as normal functions than to
// test it compile-time with some unit testing framework as this way we rule out architectural
// differences better. Tests will start by logging test starts to error (when #DEBUG is set!)
// and info logs and the method returns false when anything has failed...
    static bool TEST_FacePoint() {
#ifdef DEBUG
        OMLOGE("TEST_FacePoint...");
#endif
        // should parse
        const char *vnetest = "1/1/1";
        const char *vnetest2 = "2/2/3";
        // should not parse
        const char *vnetest3 = "v 1.0 2.0 3.0";
        const char *vnetest4 = "asdafdas";
        const char *vnetest5 = nullptr;
        const char *vnetest6 = "";

        OMLOGI("TEST_FacePoint testing: %s", vnetest);
        if(FacePoint::isParsable(vnetest)) {
            FacePoint fp(vnetest);
#ifdef DEBUG
            OMLOGE("facepoint: %d/%d/%d", fp.vIndex, fp.vtIndex, fp.vnIndex);
            OMLOGE("facepoint.asText(): %s", fp.asText().c_str();
#endif
            // indexing is from 1 in the file - we use indexing from zero!
            if(fp.vIndex != 0) { OMLOGE("Bad vIndex value: %d", fp.vIndex); return false; }
            if(fp.vtIndex != 0) { OMLOGE("Bad vtIndex value: %d", fp.vtIndex); return false; }
            if(fp.vnIndex != 0) { OMLOGE("Bad vnIndex value: %d", fp.vnIndex); return false; }
        } else {
            OMLOGE("Cannot parse: %s", vnetest);
            return false;
        }

        OMLOGI("TEST_FacePoint testing(force-parse): %s", vnetest2);
        if(FacePoint::isParsable(vnetest2)) {
            FacePoint fp(vnetest2, true);
#ifdef DEBUG
            OMLOGE("facepoint: %d/%d/%d", fp.vIndex, fp.vtIndex, fp.vnIndex);
#endif
            // indexing is from 1 in the file - we use indexing from zero!
            if(fp.vIndex != 1) { OMLOGE("Bad vIndex value: %d", fp.vIndex); return false; }
            if(fp.vtIndex != 1) { OMLOGE("Bad vtIndex value: %d", fp.vtIndex); return false; }
            if(fp.vnIndex != 2) { OMLOGE("Bad vnIndex value: %d", fp.vnIndex); return false; }
        } else {
            OMLOGE("Cannot parse: %s", vnetest2);
            return false;
        }

        OMLOGI("TEST_FacePoint testing: %s", vnetest3);
        if(FacePoint::isParsable(vnetest3)) {
            OMLOGE("Mistakenly parsed: %s", vnetest3);
        }

        OMLOGI("TEST_FacePoint testing: %s", vnetest4);
        if(FacePoint::isParsable(vnetest4)) {
            OMLOGE("Mistakenly parsed: %s", vnetest4);
        }

        OMLOGI("TEST_FacePoint testing: %s", vnetest5);
        if(FacePoint::isParsable(vnetest5)) {
            OMLOGE("Mistakenly parsed: %s", vnetest5);
        }

        OMLOGI("TEST_FacePoint testing: %s", vnetest6);
        if(FacePoint::isParsable(vnetest6)) {
            OMLOGE("Mistakenly parsed: %s", vnetest6);
        }

#ifdef DEBUG
        OMLOGE("...TEST_FacePoint completed (OK)");
#endif
        return true;
    }
}
#endif //NFTSIMPLEPROJ_FACEPOINT_H
