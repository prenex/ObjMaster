//
// Created by rthier on 2016.04.08..
//
#ifndef NFTSIMPLEPROJ_VERTEXELEMENT_H
#define NFTSIMPLEPROJ_VERTEXELEMENT_H

#include <string>

#include "objmasterlog.h"

namespace ObjMaster {

    class VertexElement final {
    public:
        float x;
        float y;
        float z;

	// Copies are defeaulted
	VertexElement(const VertexElement &other) = default;
	VertexElement& operator=(const VertexElement &other) = default;
	// Moves are defaulted
	VertexElement(VertexElement &&other) = default;
	VertexElement& operator=(VertexElement &&other) = default;

        VertexElement() {};

        VertexElement(float xx, float yy, float zz) { x = xx; y = yy; z = zz; };

        VertexElement(char *fields);

        VertexElement(const char *fields);

        static bool isParsable(const char *fields);

	/** Gets the textual representation */
	inline std::string asText() {
		return "v " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
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
    static int TEST_VertexElement_Output(){
        const char *vetest = "v 1.0 2.0 3.0";
	// Parse
	VertexElement v(vetest);
	// Get as string
	auto str = v.asText();
	// Reparse result
	VertexElement v2(str.c_str());
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

    static bool TEST_VertexElement() {
#ifdef DEBUG
        OMLOGI("TEST_VertexElement...");
#endif
        const char *vetest = "v 1.0 2.0 3.0";
        const char *vetest2 = "vt 1.0 2.0 3.0";
        const char *vetest3 = "vn 1.0 2.0 3.0";
        const char *vetest4 = "f 1/1/1 2/2/2 3/3/3";
        const char *vetest5 = nullptr;
        const char *vetest6 = "";

        OMLOGI("TEST_VertexElement testing: %s", vetest);
        if(VertexElement::isParsable(vetest)) {
            VertexElement v(vetest);
#ifdef DEBUG
            OMLOGI("v: (%f,%f,%f)", v.x, v.y, v.z);
            OMLOGI("v.asText(): %s", v.asText().c_str());
#endif
            if(v.x != 1.0) { OMLOGE("Bad x value: %f", v.x); return false; }
            if(v.y != 2.0) { OMLOGE("Bad y value: %f", v.y); return false; }
            if(v.z != 3.0) { OMLOGE("Bad z value: %f", v.z); return false; }
        } else {
            OMLOGE("Cannot parse: %s", vetest);
            return false;
        }

        OMLOGI("TEST_VertexElement testing: %s", vetest2);
        if(VertexElement::isParsable(vetest2)) {
            OMLOGE("Mistakenly parsed: %s", vetest2);
        }

        OMLOGI("TEST_VertexElement testing: %s", vetest3);
        if(VertexElement::isParsable(vetest3)) {
            OMLOGE("Mistakenly parsed: %s", vetest3);
        }

        OMLOGI("TEST_VertexElement testing: %s", vetest4);
        if(VertexElement::isParsable(vetest4)) {
            OMLOGE("Mistakenly parsed: %s", vetest4);
        }

        OMLOGI("TEST_VertexElement testing: %s", vetest5);
        if(VertexElement::isParsable(vetest5)) {
            OMLOGE("Mistakenly parsed: %s", vetest5);
        }

        OMLOGI("TEST_VertexElement testing: %s", vetest6);
        if(VertexElement::isParsable(vetest6)) {
            OMLOGE("Mistakenly parsed: %s", vetest6);
        }

#ifdef DEBUG
        OMLOGI("...TEST_VertexElement completed (OK)");
#endif
        return true;
    }
}
#endif //NFTSIMPLEPROJ_VERTEXELEMENT_H
