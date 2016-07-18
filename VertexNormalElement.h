//
// Created by rthier on 2016.04.11..
//

#ifndef NFTSIMPLEPROJ_VERTEXNORMALELEMENT_H
#define NFTSIMPLEPROJ_VERTEXNORMALELEMENT_H

#include "objmasterlog.h"

namespace ObjMaster {

    class VertexNormalElement final {
    public:
        float x;
        float y;
        float z;

        VertexNormalElement() {};

        VertexNormalElement(char *fields);

        VertexNormalElement(const char *fields);

        static bool isParsable(const char *fields);
    private:
        void constructionHelper(char *fields);
    };

// Very simple unit-testing approach
// it is better to have these run on the device itself and callable as normal functions than to
// test it compile-time with some unit testing framework as this way we rule out architectural
// differences better. Tests will start by logging test starts to error (when #DEBUG is set!)
// and info logs and the method returns false when anything has failed...
    static bool TEST_VertexNormalElement() {
#ifdef DEBUG
        OMLOGE("TEST_VertexNormalElement...");
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
            OMLOGE("vn: (%f,%f,%f)", v.x, v.y, v.z);
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
        OMLOGE("...TEST_VertexNormalElement completed (OK)");
#endif
        return true;
    }
}
#endif //NFTSIMPLEPROJ_VERTEXNORMALELEMENT_H
