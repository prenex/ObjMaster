//
// Created by rthier on 2016.04.08..
//
#ifndef NFTSIMPLEPROJ_VERTEXELEMENT_H
#define NFTSIMPLEPROJ_VERTEXELEMENT_H

#include "objmasterlog.h"

namespace ObjMaster {

    class VertexElement final {
    public:
        float x;
        float y;
        float z;

        VertexElement() {};

        VertexElement(char *fields);

        VertexElement(const char *fields);

        static bool isParsable(const char *fields);
    private:
        void constructionHelper(char *fields);
    };

// Very simple unit-testing approach
// it is better to have these run on the device itself and callable as normal functions than to
// test it compile-time with some unit testing framework as this way we rule out architectural
// differences better. Tests will start by logging test starts to error (when #DEBUG is set!)
// and info logs and the method returns false when anything has failed...
    static bool TEST_VertexElement() {
#ifdef DEBUG
        OMLOGE("TEST_VertexElement...");
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
            OMLOGE("v: (%f,%f,%f)", v.x, v.y, v.z);
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
        OMLOGE("...TEST_VertexElement completed (OK)");
#endif
        return true;
    }
}
#endif //NFTSIMPLEPROJ_VERTEXELEMENT_H
