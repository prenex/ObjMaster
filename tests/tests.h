// Contains some simple global test cases for various things

#ifndef OBJMASTER_TESTS_H
#define OBJMASTER_TESTS_H

// Useful to test memory usage runtime in POSIX systems
#if defined (__linux__) || defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <sys/resource.h>
#endif // getrusage

// Some headers we might test out
#include "../objmasterlog.h"
#include "../Obj.h"
#include "../ObjMeshObject.h"
#include "../MaterializedObjModel.h"
#include "../objmasterlog.h"
#include "../FileAssetLibrary.h"
#include "../StbImgTexturePreparationLibrary.h"
#include "../ext/GlGpuTexturePreparationLibrary.h"

// For output testing of elements
#include "../VertexElement.h"
#include "../VertexTextureElement.h"
#include "../VertexNormalElement.h"
#include "../FaceElement.h"
#include "../UseMtl.h"
#include "../LineElement.h"
#include "../ObjectGroupElement.h"
#include "../Material.h"

// At least we can try testing the facade as if we would see it from plain C...
#include "../ext/integration/ObjMasterIntegrationFacade.h"

namespace ObjMasterTest {

	const char* TEST_MODEL_PATH = "objmaster/tests/models/";
	const char* TEST_MODEL = "test.obj";
	const int INNER_LOOPS = 16;
	const int OUTER_LOOPS = 4;

	/** Gets the used bytes where this is properly implemented. Many linux kernels still don't have this */
	long getMemoryUsage() {
#if defined (__linux__) || defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
		struct rusage r_usage;
		getrusage(RUSAGE_SELF,&r_usage);
		return r_usage.ru_maxrss;
#else
		OMLOGE("Cannot guess memory usage, just returning zero!");
		return 0;
#endif // getrusage
	}

	/** Tests the integration facade functionality. Returns 0 if everything is successful and 1 on errors */
	int testIntegrationFacade(){
		int handle = loadObjModel(TEST_MODEL_PATH, TEST_MODEL);
		int meshNo = getModelMeshNo(handle);
		if(meshNo < 1) {
			// No meshes have found in the test model. This is an error.
			OMLOGE("No meshes have found in the model (%d mesh count: %d)", handle, meshNo);
			return 1;
		} else {
			// We have found some meshes
			OMLOGI("Number of found meshes in %d is: %d", handle, meshNo);
			int vertexNo = getModelMeshVertexDataCount(handle, 0);
			OMLOGI("Number of found vertices in first mesh of %d is: %d", handle, vertexNo);
			bool success = unloadObjModel(handle);
			if(!success) {
				OMLOGE("Error while model(%d) unload!", handle);
				return 1;
			} else {
				// 0 errors
				return 0;
			}
		}
	}
	
	/** Used in testMemoryLeakage */
	int leakTest1() {
		// Get memory usage in the beginning
		OMLOGI("LEAKTEST 1) Memory at start: %ld", getMemoryUsage());
		for(int i = 0; i < OUTER_LOOPS; ++i) {
			testIntegrationFacade();
			OMLOGI("LEAKTEST 1) Memory at iteration %d: %ld", i, getMemoryUsage());
		}
		// Get memory usage in the end
		OMLOGI("LEAKTEST 1) Memory at end of leakage test: %ld", getMemoryUsage());

		// This should really unload everything
		unloadEverything();

		// Get memory usage in the very end
		OMLOGI("LEAKTEST 1) Memory at very end of leakage test: %ld", getMemoryUsage());

		// TODO: ensure mem is the same?
		return 0;
	}

	/** Used in testMemoryLeakage */
	int leakTest2() {
		// Get memory usage in the beginning
		OMLOGI("LEAKTEST 2) Memory at start: %ld", getMemoryUsage());
		// Try doing a lot of Obj parses
		for(int i = 0; i < OUTER_LOOPS; ++i) {
			for(int j = 0; j < INNER_LOOPS; ++j) {
				ObjMaster::Obj obj = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), TEST_MODEL_PATH, TEST_MODEL);

				int verticesNo = obj.vs.size();
				OMLOGI("Number of vertices in test: %d", verticesNo);
			}
			OMLOGI("LEAKTEST 2) Memory at iteration %d: %ld", i, getMemoryUsage());
		}
		// Get memory usage in the end
		OMLOGI("LEAKTEST 2) Memory at end of leakage test: %ld", getMemoryUsage());

		// TODO: ensure mem is the same?
		return 0;
	}

	/** Used in testMemoryLeakage */
	int leakTest3() {
		// Get memory usage in the beginning
		OMLOGI("LEAKTEST 3) Memory at start: %ld", getMemoryUsage());
		// Parse the obj once
		ObjMaster::Obj obj = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), TEST_MODEL_PATH, TEST_MODEL);
		// Then try to create a local objects on the stack with copy constructor
		for(int i = 0; i < OUTER_LOOPS; ++i) {
			for(int j = 0; j < INNER_LOOPS; ++j) {
				ObjMaster::Obj copyobj = obj;
				int verticesNo = copyobj.vs.size();
				OMLOGI("Number of vertices in test: %d", verticesNo);
			}
			OMLOGI("LEAKTEST 3) Memory at iteration %d: %ld", i, getMemoryUsage());
		}
		// Get memory usage in the end
		OMLOGI("LEAKTEST 3) Memory at end of leakage test: %ld", getMemoryUsage());

		// TODO: ensure mem is the same?
		return 0;
	}

	/** Used in testMemoryLeakage */
	int leakTest4() {
		// Get memory usage in the beginning
		OMLOGI("LEAKTEST 4) Memory at start: %ld", getMemoryUsage());
		// Then try to create a local objects on the stack with copy constructor
		for(int i = 0; i < OUTER_LOOPS; ++i) {
			for(int j = 0; j < INNER_LOOPS; ++j) {
				// Test parts of obj - basically parsing elements memory test
				ObjMaster::TEST_Obj();
			}
			OMLOGI("LEAKTEST 4) Memory at iteration %d: %ld", i, getMemoryUsage());
		}
		// Get memory usage in the end
		OMLOGI("LEAKTEST 4) Memory at end of leakage test: %ld", getMemoryUsage());

		// TODO: ensure mem is the same?
		return 0;
	}

	/** test memory leaks - logs the usage while testing */
	int testMemoryLeakage() {
		int errorCount = 0;

		// Test leakage of integration facade
		errorCount += leakTest1();
		errorCount += leakTest2();
		errorCount += leakTest3();
		errorCount += leakTest4();

		// TODO: ensure mem is the same?
		return 0;
	}

	/** Tests the "asText()" calls to the various *Element classes */
	int testElementOutputs() {
		// Init error count
		int errorCount = 0;

		// Test all elements
		errorCount += ObjMaster::TEST_VertexElement_Output();
		errorCount += ObjMaster::TEST_VertexTextureElement_Output();
		errorCount += ObjMaster::TEST_VertexNormalElement_Output();
		errorCount += ObjMaster::TEST_FaceElement_Output();
		errorCount += ObjMaster::TEST_UseMtl_Output();
		errorCount += ObjMaster::TEST_LineElement_Output();
		errorCount += ObjMaster::TEST_ObjectGroupElement_Output();
		errorCount += ObjMaster::TEST_Material_Output();
		
		// Return error count
		return errorCount;
	}

	/** Run all tests for testing *.obj output methods */
	int testObjOutput() {
		OMLOGI("OBJ OUTPUT TEST...");
		// Init error count
		int errorCount = 0;

		// Run all related tests
		errorCount += testElementOutputs();
		
		OMLOGI("...OBJ OUTPUT TEST had %d errors", errorCount);
		// Return error count
		return errorCount;
	}

	/** Run all tests and return the number of error cases */
	int testAll() {
		// Init error count
		int errorCount = 0;
		// Run all tests
		errorCount += testIntegrationFacade();
		errorCount += testMemoryLeakage();
		errorCount += testObjOutput();
		// Return error count
		return errorCount;
	}
}

#endif // OBJMASTER_TESTS_H
