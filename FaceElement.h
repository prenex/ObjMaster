//
// Created by rthier on 2016.04.11..
//

#ifndef NFTSIMPLEPROJ_FACEELEMENT_H
#define NFTSIMPLEPROJ_FACEELEMENT_H

#include "FacePoint.h"
#include "objmasterlog.h"

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

    private:
        // Common parts of constructors
        void constructionHelper(char* fields);
    };

    // Very simple unit-testing approach
    // it is better to have these run on the device itself and callable as normal functions than to
    // test it compile-time with some unit testing framework as this way we rule out architectural
    // differences better. Tests will start by logging test starts to error (when #DEBUG is set!)
    // and info logs and the method returns false when anything has failed...
    static bool TEST_FaceElement() {
#ifdef DEBUG
        OMLOGE("TEST_FaceElement...");
#endif
        // Should parse
        const char *vnetest = "f 1/2/3 4/5/6 7/8/9";
        // Should not parse
        const char *vnetest2 = "vn 1.0 2.0 3.0";
        const char *vnetest3 = "v 1.0 2.0 3.0";
        const char *vnetest4 = "vt 1.0 2.0";
        const char *vnetest5 = nullptr;
        const char *vnetest6 = "";

        OMLOGI("TEST_FaceElement testing: %s", vnetest);
        if(FaceElement::isParsable(vnetest)) {
            FaceElement f(vnetest);
#ifdef DEBUG
            OMLOGE("f(facePointCount): %d", f.facePointCount);
#endif
            if(f.facePointCount != 3) { OMLOGE("Bad facePointCount value: %d", f.facePointCount); return false; }

            if(f.facePoints[0].vnIndex != 2) { OMLOGE("Bad vnIndex value: %d", f.facePoints[0].vnIndex); return false; }
            if(f.facePoints[1].vnIndex != 5) { OMLOGE("Bad vnIndex value: %d", f.facePoints[1].vnIndex); return false; }
            if(f.facePoints[0].vIndex != 0) { OMLOGE("Bad vIndex value: %d", f.facePoints[0].vIndex); return false; }
            if(f.facePoints[2].vtIndex != 7) { OMLOGE("Bad vtIndex value: %d", f.facePoints[0].vtIndex); return false; }
        } else {
            OMLOGE("Cannot parse: %s", vnetest);
            return false;
        }

        OMLOGI("TEST_FaceElement testing: %s", vnetest2);
        if(FaceElement::isParsable(vnetest2)) {
            OMLOGE("Mistakenly parsed: %s", vnetest2);
        }

        OMLOGI("TEST_FaceElement testing: %s", vnetest3);
        if(FaceElement::isParsable(vnetest3)) {
            OMLOGE("Mistakenly parsed: %s", vnetest3);
        }

        OMLOGI("TEST_FaceElement testing: %s", vnetest4);
        if(FaceElement::isParsable(vnetest4)) {
            OMLOGE("Mistakenly parsed: %s", vnetest4);
        }

        OMLOGI("TEST_FaceElement testing: %s", vnetest5);
        if(FaceElement::isParsable(vnetest5)) {
            OMLOGE("Mistakenly parsed: %s", vnetest5);
        }

        OMLOGI("TEST_FaceElement testing: %s", vnetest6);
        if(FaceElement::isParsable(vnetest6)) {
            OMLOGE("Mistakenly parsed: %s", vnetest6);
        }

#ifdef DEBUG
        OMLOGE("...TEST_FaceElement completed (OK)");
#endif
        return true;
    }
}
#endif //NFTSIMPLEPROJ_FACEELEMENT_H
