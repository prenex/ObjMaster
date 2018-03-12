//
// Created by rthier on 2016.04.11..
//

#ifndef NFTSIMPLEPROJ_VERTEXTEXTUREELEMENT_H
#define NFTSIMPLEPROJ_VERTEXTEXTUREELEMENT_H

#include "objmasterlog.h"
#include <string>

namespace ObjMaster {

    class VertexTextureElement final {
    public:
        float u;
        float v;

        VertexTextureElement() {};

        VertexTextureElement(float uu, float vv) {u = uu; v = vv;};

        VertexTextureElement(char *fields);

        VertexTextureElement(const char *fields);

        static bool isParsable(const char *fields);

	/** Gets the textual representation */
	inline std::string asText() {
		return "vt " + std::to_string(u) + " " + std::to_string(v);
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
    static int TEST_VertexTextureElement_Output(){
        const char *vetest = "vt 1.0 2.0";
	// Parse
	VertexTextureElement v(vetest);
	// Get as string
	auto str = v.asText();
	// Reparse result
	VertexTextureElement v2(str.c_str());
	// Compare original and reparsed - this should test output reasonably well
	if((v.u == v2.u) && (v.v == v2.v)) {
		// OK
		return 0;
	} else {
		// ERROR
		OMLOGE("Bad VertexTextureElement output: %s instead of %s", str.c_str(), vetest);
		return 1;
	}
    }
    static bool TEST_VertexTextureElement() {
#ifdef DEBUG
        OMLOGI("TEST_VertexTextureElement...");
#endif
        const char *vnetest = "vt 1.0 2.0";
        const char *vnetest2 = "vn 1.0 2.0 3.0";
        const char *vnetest3 = "v 1.0 2.0 3.0";
        const char *vnetest4 = "f 1/1/1 2/2/2 3/3/3";
        const char *vnetest5 = nullptr;
        const char *vnetest6 = "";

        OMLOGI("TEST_VertexTextureElement testing: %s", vnetest);
        if(VertexTextureElement::isParsable(vnetest)) {
            VertexTextureElement v(vnetest);
#ifdef DEBUG
            OMLOGI("vt: (%f,%f)", v.u, v.v);
            OMLOGI("vt.asText(): %s", v.asText().c_str());
#endif
            if(v.u != 1.0) { OMLOGE("Bad u value: %f", v.u); return false; }
            if(v.v != 2.0) { OMLOGE("Bad v value: %f", v.v); return false; }
        } else {
            OMLOGE("Cannot parse: %s", vnetest);
            return false;
        }

        OMLOGI("TEST_VertexTextureElement testing: %s", vnetest2);
        if(VertexTextureElement::isParsable(vnetest2)) {
            OMLOGE("Mistakenly parsed: %s", vnetest2);
        }

        OMLOGI("TEST_VertexTextureElement testing: %s", vnetest3);
        if(VertexTextureElement::isParsable(vnetest3)) {
            OMLOGE("Mistakenly parsed: %s", vnetest3);
        }

        OMLOGI("TEST_VertexTextureElement testing: %s", vnetest4);
        if(VertexTextureElement::isParsable(vnetest4)) {
            OMLOGE("Mistakenly parsed: %s", vnetest4);
        }

        OMLOGI("TEST_VertexTextureElement testing: %s", vnetest5);
        if(VertexTextureElement::isParsable(vnetest5)) {
            OMLOGE("Mistakenly parsed: %s", vnetest5);
        }

        OMLOGI("TEST_VertexTextureElement testing: %s", vnetest6);
        if(VertexTextureElement::isParsable(vnetest6)) {
            OMLOGE("Mistakenly parsed: %s", vnetest6);
        }

#ifdef DEBUG
        OMLOGI("...TEST_VertexTextureElement completed (OK)");
#endif
        return true;
    }
}
#endif //NFTSIMPLEPROJ_VERTEXTEXTUREELEMENT_H
