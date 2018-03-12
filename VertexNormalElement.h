//
// Created by rthier on 2016.04.11..
//

#ifndef NFTSIMPLEPROJ_VERTEXNORMALELEMENT_H
#define NFTSIMPLEPROJ_VERTEXNORMALELEMENT_H

#include "objmasterlog.h"
#include <string>

namespace ObjMaster {

    class VertexNormalElement final {
    public:
        float x;
        float y;
        float z;

        VertexNormalElement() {};

        VertexNormalElement(float xx, float yy, float zz) { x = xx; y = yy; z = zz; };

        VertexNormalElement(char *fields);

        VertexNormalElement(const char *fields);

        static bool isParsable(const char *fields);
	/** Gets the textual representation */
	inline std::string asText() {
		return "vn " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
	}
    private:
        void constructionHelper(char *fields);
    };

// Very simple unit-testing approach
// it is better to have these run on the device itself and callable as normal functions than to
// test it compile-time with some unit testing framework as this way we rule out architectural
// differences better. Tests will start by logging test starts to error (when #DEBUG is set!)
// and info logs and the method returns false when anything has failed...
    /** testing output-related operations (like asText()) */
    static int TEST_VertexNormalElement_Output(){
        const char *vetest = "vn 1.0 2.0 3.0";
	// Parse
	VertexNormalElement v(vetest);
	// Get as string
	auto str = v.asText();
	// Reparse result
	VertexNormalElement v2(str.c_str());
	// Compare original and reparsed - this should test output reasonably well
	if((v.x == v2.x) && (v.y == v2.y) && (v.z == v2.z)) {
		// OK
		return 0;
	} else {
		// ERROR
		OMLOGE("Bad VertexElement output: %s instead of %s", str.c_str(), vetest);
		return 1;
	}
    }
    static bool TEST_VertexNormalElement() {
#ifdef DEBUG
        OMLOGI("TEST_VertexNormalElement...");
#endif
        const char *vnetest = "vn 1.0 2.0 3.0";
        const char *vnetest2 = "vt 1.0 2.0 3.0";
        const char *vnetest3 = "v 1.0 2.0 3.0";
        const char *vnetest4 = "f 1/1/1 2/2/2 3/3/3";
        const char *vnetest5 = nullptr;
        const char *vnetest6 = "";

        OMLOGI("TEST_VertexNormalElement testing: %s", vnetest);
        if(VertexNormalElement::isParsable(vnetest)) {
            VertexNormalElement v(vnetest);
#ifdef DEBUG
            OMLOGI("vn: (%f,%f,%f)", v.x, v.y, v.z);
            OMLOGI("vn.asText(): %s", v.asText().c_str());
#endif
            if(v.x != 1.0) { OMLOGE("Bad x value: %f", v.x); return false; }
            if(v.y != 2.0) { OMLOGE("Bad y value: %f", v.y); return false; }
            if(v.z != 3.0) { OMLOGE("Bad z value: %f", v.z); return false; }
        } else {
            OMLOGE("Cannot parse: %s", vnetest);
            return false;
        }

        OMLOGI("TEST_VertexNormalElement testing: %s", vnetest2);
        if(VertexNormalElement::isParsable(vnetest2)) {
            OMLOGE("Mistakenly parsed: %s", vnetest2);
        }

        OMLOGI("TEST_VertexNormalElement testing: %s", vnetest3);
        if(VertexNormalElement::isParsable(vnetest3)) {
            OMLOGE("Mistakenly parsed: %s", vnetest3);
        }

        OMLOGI("TEST_VertexNormalElement testing: %s", vnetest4);
        if(VertexNormalElement::isParsable(vnetest4)) {
            OMLOGE("Mistakenly parsed: %s", vnetest4);
        }

        OMLOGI("TEST_VertexNormalElement testing: %s", vnetest5);
        if(VertexNormalElement::isParsable(vnetest5)) {
            OMLOGE("Mistakenly parsed: %s", vnetest5);
        }

        OMLOGI("TEST_VertexNormalElement testing: %s", vnetest6);
        if(VertexNormalElement::isParsable(vnetest6)) {
            OMLOGE("Mistakenly parsed: %s", vnetest6);
        }

#ifdef DEBUG
        OMLOGI("...TEST_VertexNormalElement completed (OK)");
#endif
        return true;
    }
}
#endif //NFTSIMPLEPROJ_VERTEXNORMALELEMENT_H
