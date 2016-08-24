//
// Created by rthier on 2016.04.11..
//

#ifndef NFTSIMPLEPROJ_VERTEXTEXTUREELEMENT_H
#define NFTSIMPLEPROJ_VERTEXTEXTUREELEMENT_H

#include "objmasterlog.h"

namespace ObjMaster {

    class VertexTextureElement final {
    public:
        float u;
        float v;

        VertexTextureElement() {};

        VertexTextureElement(char *fields);

        VertexTextureElement(const char *fields);

        static bool isParsable(const char *fields);
    private:
        void constructionHelper(char *fields);
    };

// Very simple unit-testing approach
// it is better to have these run on the device itself and callable as normal functions than to
// test it compile-time with some unit testing framework as this way we rule out architectural
// differences better. Tests will start by logging test starts to error (when #DEBUG is set!)
// and info logs and the method returns false when anything has failed...
    static bool TEST_VertexTextureElement() {
#ifdef DEBUG
        OMLOGE("TEST_VertexTextureElement...");
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
            OMLOGE("vt: (%f,%f)", v.u, v.v);
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
        OMLOGE("...TEST_VertexTextureElement completed (OK)");
#endif
        return true;
    }
}
#endif //NFTSIMPLEPROJ_VERTEXTEXTUREELEMENT_H
