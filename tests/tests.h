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
#include "../ObjCreator.h"

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

	const char* EXT_TEST_MODEL_PATH = "objmaster/examples/extmodels/";
	const char* TEST_MODEL_PATH = "objmaster/tests/models/";
	const char* EXT_TEST_MODEL = "unityexport.obj"; //"test.obj";
	const char* TEST_MODEL = "Rose_flower_tex3.obj"; //"test.obj";
	const char* TEST_OUT_PATH = "";
	const char* TEST_OUT_MTL = "out.mtl"; // This is to separate MtlLib.saveAs(..) testing from the full round-trip test
	// Rem.: this will have its test_out.mtl too
	const char* TEST_OUT_MODEL = "test_out.obj";
	const char* GENERATED_TEST_OUT_MODEL = "gen_test_out.obj";
	const char* GENERATED_APPEND_TEST_OUT_MODEL = "gen_test_append_out.obj";
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

	/** Used in testMemoryLeakage - test move constructor */
	int leakTest3() {
		// Get memory usage in the beginning
		OMLOGI("LEAKTEST 3) Memory at start: %ld", getMemoryUsage());
		// Parse the obj once
		ObjMaster::Obj obj = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), TEST_MODEL_PATH, TEST_MODEL);
		// Then try to create a local objects on the stack with move constructor
		for(int i = 0; i < OUTER_LOOPS; ++i) {
			for(int j = 0; j < INNER_LOOPS; ++j) {
				auto mobj = std::move(obj);
				int verticesNo = mobj.vs.size();
				OMLOGI("Number of vertices in test: %d", verticesNo);
				// "move back"
				obj = std::move(mobj);
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

	/** Tests for saving *.mtl files */
	int testMtlLibSave() {
		OMLOGI("Testing *.mtl saving...");
		// Parse the test model
		ObjMaster::Obj obj = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), TEST_MODEL_PATH, TEST_MODEL);
		// Save out its mtl file in the current directory
		obj.mtlLib.saveAs(ObjMaster::FileAssetLibrary(), TEST_OUT_MTL);
		// Notify user about the saved file
		OMLOGI("See %s for the results!", TEST_OUT_MTL);

		OMLOGI("...tested *.mtl saving!");
		// This testing is only here for manual tests as of now...
		return 0;
	}

	/** Full I/O round-trip testing with a real model and also some generated data! */
	int testObjSaveAndCreate() {
		OMLOGI("Testing *.obj (re-)saving...");
		// Parse the test model
		ObjMaster::Obj obj = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), TEST_MODEL_PATH, TEST_MODEL);

		// Add a new material to the *.mtl file generated back
		ObjMaster::Material m("runtime_added_material");
		m.setAndEnableKd({1.0f, 0.0f, 0.0f});
		m.setAndEnableKs({0.0f, 1.0f, 0.0f});
		m.setAndEnableMapKd("UV_exampl_3_C.png");
		obj.mtlLib.addRuntimeGeneratedMaterial(m);

		// Save out its *.obj and *.mtl in the current directory
		obj.saveAs(ObjMaster::FileAssetLibrary(), TEST_MODEL_PATH, TEST_OUT_MODEL);
		OMLOGI("See %s for the results!", TEST_OUT_MODEL);

		OMLOGI("...tested *.obj (re-)saving!");

		OMLOGI("Testing *.obj creation and saving...");
		// TODO: Test these when they are implemented
		OMLOGI("...tested *.obj creation and saving!");

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

	ObjMaster::Obj createRuntimeTestObj() {
		ObjMaster::ObjCreator creator;
		ObjMaster::Material red;
		red.setAndEnableKd({1.0f, 0.0f, 0.0f, 1.0f});
		red.setAndEnableMapKd("wall_diff.jpg");
		red.setAndEnableMapBump("wall_norm.jpg");
		red.name = "red";
		ObjMaster::Material blue;
		blue.setAndEnableKd({0.0f, 0.0f, 1.0f, 1.0f});
		blue.setAndEnableMapKd("wall_diff.jpg");
		blue.setAndEnableMapBump("wall_norm.jpg");
		blue.name = "blue";

		creator.addRuntimeGeneratedMaterial(red);
		creator.addRuntimeGeneratedMaterial(blue);

		// Create a quad with UV and normals - facing upwards
		// This example demonstates well how one is usually exporting
		// data already prepared for rendering (like data from index and VAO buffers)

		// B C
		// A D
		// ---
		// A
		creator.unsafeAddVertexStructure(
				VertexStructure{
					0, 0, 0,
					0, 1, 0,
					0, 0
				}
		);
		// B
		creator.unsafeAddVertexStructure(
				VertexStructure{
					0, 0, 1,
					0, 1, 0,
					0, 1
				}
		);
		// C
		creator.unsafeAddVertexStructure(
				VertexStructure{
					1, 0, 1,
					0, 1, 0,
					1, 1
				}
		);
		// D
		creator.unsafeAddVertexStructure(
				VertexStructure{
					1, 0, 0,
					0, 1, 0,
					1, 0
				}
		);

		// ABC triangle
		creator.useMaterial("red");
		creator.addFace(0, 1, 2);
		// ACD triangle
		creator.useMaterial("blue");
		creator.addFace(0, 2, 3);

		// Create the box - in this case with dense obj representation
		// This is a good example to show how to export and generate Objs with
		// tricks and a lot of shared data that will be exported int a compact presentation

		// Test copy of material - this must share textures
		ObjMaster::Material wall = red;
		wall.setAndEnableKd({1.0f, 1.0f, 1.0f, 1.0f});
		wall.name = "wall";
		// Add this material to the material library
		creator.addRuntimeGeneratedMaterial(wall);
		// Use this material for the whole box
		creator.useMaterial("wall");
		creator.useGroup("dense_box");
		// The bottom 4 points
		int b1 = 0; //Re-use first vertex from above! Alternative: int b1 = creator.llAddVertex(0, 0, 0);
		int b2 = creator.llAddVertex(0, 0, 0.5f);
		int b3 = creator.llAddVertex(0.5f, 0, 0.5f);
		int b4 = creator.llAddVertex(0.5f, 0, 0);
		// The top 4 points
		int t1 = creator.llAddVertex(0, 0.5f, 0);
		int t2 = creator.llAddVertex(0, 0.5f, 0.5f);
		int t3 = creator.llAddVertex(0.5f, 0.5f, 0.5f);
		int t4 = creator.llAddVertex(0.5f, 0.5f, 0);
		// normal-vectors
		int up = 0; // Re-use the normal defined above already
		int left = creator.llAddVertexNormal(-1, 0, 0);
		int right = creator.llAddVertexNormal(1, 0, 0);
		int down = creator.llAddVertexNormal(0, -1, 0);
		int frwd = creator.llAddVertexNormal(0, 0, 1);
		int back = creator.llAddVertexNormal(0, 0, -1);
		// uvs
		int uv1 = 0; // Re-use the already existing UV-s
		int uv2 = 1;
		int uv3 = 2;
		int uv4 = 3;
		// Add faces - this is a good example to use named data when generating!
		// down1
		creator.addFaceNonHomogenous(
				b1, uv1, down,
				b3, uv2, down,
				b2, uv3, down
		);
		// down2
		creator.addFaceNonHomogenous(
				b1, uv1, down,
				b4, uv3, down,
				b3, uv4, down
		);
		// top1
		creator.addFaceNonHomogenous(
				t1, uv1, up,
				t2, uv2, up,
				t3, uv3, up
		);
		// top2
		creator.addFaceNonHomogenous(
				t1, uv1, up,
				t3, uv3, up,
				t4, uv4, up
		);
		// front1
		creator.addFaceNonHomogenous(
				b1, uv1, back,
				t1, uv2, back,
				t4, uv3, back
		);
		// front2
		creator.addFaceNonHomogenous(
				b1, uv1, back,
				t4, uv3, back,
				b4, uv4, back
		);
		// back1
		creator.addFaceNonHomogenous(
				b2, uv1, frwd,
				t3, uv2, frwd,
				t2, uv3, frwd
		);
		// back2
		creator.addFaceNonHomogenous(
				b2, uv1, frwd,
				b3, uv3, frwd,
				t3, uv4, frwd
		);
		// left1
		creator.addFaceNonHomogenous(
				b2, uv1, left,
				t2, uv2, left,
				t1, uv3, left
		);
		// left2
		creator.addFaceNonHomogenous(
				b2, uv1, left,
				t1, uv3, left,
				b1, uv4, left
		);
		// right1
		creator.addFaceNonHomogenous(
				b4, uv1, right,
				t4, uv2, right,
				t3, uv3, right
		);
		// right2
		creator.addFaceNonHomogenous(
				b4, uv1, right,
				t3, uv3, right,
				b3, uv4, right
		);

		// Get a copy of the created obj
		return creator.getObj();
	}

	int testSaveRuntimeCreatedObj(ObjMaster::Obj &obj){
		// Save the obj that has been created
		obj.saveAs(ObjMaster::FileAssetLibrary(), GENERATED_TEST_OUT_MODEL);

		// TODO: do some testing with reading back the result maybe?
		return 0;
	}

	/** Parse the saved obj file; append some data; save as a third file */
	int testFacadeObjFactoryAppendResave(const char *srcName, const char *dstName) {
		// Create a factory - prefilled with data from the src file as base *.obj
		int handle = createObjFactoryWithBaseObj("", srcName);

		// Add one more quad at Y=0.5f (similar to the other at Y=0.0f in this test.h)
		// B C
		// A D
		// ---
		// A
		auto a = addHomogenousVertexStructure(
				handle,
				VertexStructure{
					0, 0.5f, 0,
					0, 1, 0,
					0, 0
				}
		);
		// B
		auto b = addHomogenousVertexStructure(
				handle,
				VertexStructure{
					0, 0.5f, 1,
					0, 1, 0,
					0, 1
				}
		);
		// C
		auto c = addHomogenousVertexStructure(
				handle,
				VertexStructure{
					1, 0.5f, 1,
					0, 1, 0,
					1, 1
				}
		);
		// D
		auto d = addHomogenousVertexStructure(
				handle,
				VertexStructure{
					1, 0.5f, 0,
					0, 1, 0,
					1, 0
				}
		);

		// Add two faces - the first "reuses" a material from the original file and the second has a new one
		// TODO: material tests here!
		int f1 = addFace(handle, a, b, c);
		int f2 = addFace(handle, a, c, d);

		// also close the factory - because we are not reusing it
		saveObjFromFactoryToFileAndPossiblyCloseFactory(handle, "", dstName, true);

		// TODO: do some better testing and stuff
		return 0;
	}

	int testObjCreator() {
		OMLOGI("Runtime *.obj generation tests...");
		// Init error count
		int errorCount = 0;

		auto obj = createRuntimeTestObj();
		errorCount += testSaveRuntimeCreatedObj(obj);
		errorCount += testFacadeObjFactoryAppendResave(GENERATED_TEST_OUT_MODEL, GENERATED_APPEND_TEST_OUT_MODEL);

		OMLOGI("...runtime *.obj generation tests had %d errors!", errorCount);
		return errorCount;
	}

	/** Run all tests for testing *.obj output methods */
	int testObjOutput() {
		OMLOGI("OBJ OUTPUT TEST...");
		// Init error count
		int errorCount = 0;

		// Run all related tests
		errorCount += testElementOutputs();
		// Test *.mtl writeout
		errorCount += testMtlLibSave();
		// Test *.obj writeout and creation
		errorCount += testObjSaveAndCreate();
		
		OMLOGI("...OBJ OUTPUT TEST had %d errors", errorCount);
		// Return error count
		return errorCount;
	}

	/** Run all tests and return the number of error cases */
	int testAll() {
		// Init error count
		int errorCount = 0;
		// Run all tests
		errorCount += testMemoryLeakage();
		errorCount += testObjOutput();
		errorCount += testObjCreator();
		errorCount += testIntegrationFacade();
		// Return sum of error counts
		return errorCount;
	}
}

#endif // OBJMASTER_TESTS_H
