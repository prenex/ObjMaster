//
// Created by rthier on 2016.04.11..
//

#ifndef NFTSIMPLEPROJ_FACEELEMENT_H
#define NFTSIMPLEPROJ_FACEELEMENT_H

#include "FacePoint.h"
#include "objmasterlog.h"
#include <string>

namespace ObjMaster {

    class FaceElement {
    public:
        int facePointCount;
        // REMARK: Currently we handle faces with at most 3 points
        // If you want to handle quads or other things, never use alloc/dealloc all the way
        // as it can result in contention for memory manager resorces and direct sizes are faster.
        // If you need to handle other cases, just make the array able to hold the longest sequence
        // that you want! The memory overuse is much less than the overhead of many release/aquire
        // in my opinion...
        static const int MAX_FACEPOINT_COUNT = 3;
        FacePoint facePoints[MAX_FACEPOINT_COUNT];

        FaceElement() {};
        FaceElement(char *fields);
        FaceElement(const char *fields);
        static bool isParsable(const char *fields);

	/** Gets the textual representation */
	inline std::string asText() {
		// Rem.: What to do if the facePointCount is 0? I better return an empty string then...
		std::string faceStr = (facePointCount > 0) ? "f " : "";
		for(int i = 0; i < facePointCount; ++i) {
			faceStr += (facePoints[i].asText() + " ");
		}
		return faceStr;
	}

    private:
        // Common parts of constructors
        void constructionHelper(char* fields);
    };

    // Very simple unit-testing approach
    // it is better to have these run on the device itself and callable as normal functions than to
    // test it compile-time with some unit testing framework as this way we rule out architectural
    // differences better. Tests will start by logging test starts to error (when #DEBUG is set!)
    // and info logs and the method returns false when anything has failed...
    /** testing output-related operations (like asText()) */
    static int TEST_FaceElement_Output(){
        const char *fetest = "f 1/2/3 4/5/6 7/8/9";
	// Parse
	FaceElement f(fetest);
	// Get as string
	auto str = f.asText();
	// Reparse result
	FaceElement fb(str.c_str());
	auto f0 = f.facePoints[0];
	auto f1 = f.facePoints[1];
	auto f2 = f.facePoints[2];
	auto fb0 = fb.facePoints[0];
	auto fb1 = fb.facePoints[1];
	auto fb2 = fb.facePoints[2];
	// Compare original and reparsed - this should test output reasonably well
	if(
		((f0.vIndex == fb0.vIndex) && (f0.vtIndex == fb0.vtIndex) && (f0.vnIndex == fb0.vnIndex)) &&
		((f1.vIndex == fb1.vIndex) && (f1.vtIndex == fb1.vtIndex) && (f1.vnIndex == fb1.vnIndex)) &&
		((f2.vIndex == fb2.vIndex) && (f2.vtIndex == fb2.vtIndex) && (f2.vnIndex == fb2.vnIndex))
	) {
		// OK
		return 0;
	} else {
		// ERROR
		OMLOGE("Bad FaceElement output: %s instead of %s", str.c_str(), fetest);
		return 1;
	}
    }
    static bool TEST_FaceElement() {
#ifdef DEBUG
        OMLOGI("TEST_FaceElement...");
#endif
        // Should parse
        const char *fetest = "f 1/2/3 4/5/6 7/8/9";
        // Should not parse
        const char *fetest2 = "vn 1.0 2.0 3.0";
        const char *fetest3 = "v 1.0 2.0 3.0";
        const char *fetest4 = "vt 1.0 2.0";
        const char *fetest5 = nullptr;
        const char *fetest6 = "";

        OMLOGI("TEST_FaceElement testing: %s", fetest);
        if(FaceElement::isParsable(fetest)) {
            FaceElement f(fetest);
#ifdef DEBUG
            OMLOGI("f(facePointCount): %d", f.facePointCount);
	    OMLOGI("Result of parse: %s", f.asText().c_str());
#endif
            if(f.facePointCount != 3) { OMLOGE("Bad facePointCount value: %d", f.facePointCount); return false; }

            if(f.facePoints[0].vnIndex != 2) { OMLOGE("Bad vnIndex value: %d", f.facePoints[0].vnIndex); return false; }
            if(f.facePoints[1].vnIndex != 5) { OMLOGE("Bad vnIndex value: %d", f.facePoints[1].vnIndex); return false; }
            if(f.facePoints[0].vIndex != 0) { OMLOGE("Bad vIndex value: %d", f.facePoints[0].vIndex); return false; }
            if(f.facePoints[2].vtIndex != 7) { OMLOGE("Bad vtIndex value: %d", f.facePoints[0].vtIndex); return false; }
        } else {
            OMLOGE("Cannot parse: %s", fetest);
            return false;
        }

        OMLOGI("TEST_FaceElement testing: %s", fetest2);
        if(FaceElement::isParsable(fetest2)) {
            OMLOGE("Mistakenly parsed: %s", fetest2);
        }

        OMLOGI("TEST_FaceElement testing: %s", fetest3);
        if(FaceElement::isParsable(fetest3)) {
            OMLOGE("Mistakenly parsed: %s", fetest3);
        }

        OMLOGI("TEST_FaceElement testing: %s", fetest4);
        if(FaceElement::isParsable(fetest4)) {
            OMLOGE("Mistakenly parsed: %s", fetest4);
        }

        OMLOGI("TEST_FaceElement testing: %s", fetest5);
        if(FaceElement::isParsable(fetest5)) {
            OMLOGE("Mistakenly parsed: %s", fetest5);
        }

        OMLOGI("TEST_FaceElement testing: %s", fetest6);
        if(FaceElement::isParsable(fetest6)) {
            OMLOGE("Mistakenly parsed: %s", fetest6);
        }

#ifdef DEBUG
        OMLOGI("...TEST_FaceElement completed (OK)");
#endif
        return true;
    }
}
#endif //NFTSIMPLEPROJ_FACEELEMENT_H
