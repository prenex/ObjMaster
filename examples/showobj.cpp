/*
 * ObjMaster/examples/showobj
 *	
 * Example obj model renderer using the library.
 * 
 * This example uses a subset of GLESv2 and EGL
 * in a way that it can be compiled both with
 * gcc, clang and emscripten (to provide webgl html)
 * 
 * Prerequisite:
 *	 freeglut3, freeglut3-dev (both) OR EGL (see makefile)
 *	 libegl1-mesa-dev, libgles2-mesa-dev (EGL/GLES2)
 *	 emsdk (JS/WEBGL - full toolchain: nodejs, LLVM, etc.)
 * Compilations:
 *	 - make
 *	 - In case you want to change the compiler/target, change the CC parameter
 *		between em++, clang++ and g++ accordingly. You can provide this to make
 *		as a command line parameter or change the top of the makefile to set the
 *		default one. The makefile has comments about tested compiler versions.
 * Usage:
 *	 ./showobj <model>		- shows the given model
 *	 ./showobj			- shows models/default.obj (as the html build)
 *	 palemoon showobj.html - open webgl build to show embedded models/default.obj
 */
#define MODEL_PATH_MAX_LEN 512

// Define this if your device only handles 16 bit indices and want to build like that!
// This means that less model data will fit in the per-group or per-mesh memory, but
// devices like Raspberry or Orange Pis and old phones will not complain. Of course 
// you can also save graphics memory this way or let the 32 bit indices work and do 
// post-processing on your own (for example you can cut a too big object to fit).
#if USE_16BIT_INDICES
#define OM_GL_INDEX_TYPE_ENUM GL_UNSIGNED_SHORT
#else
#define OM_GL_INDEX_TYPE_ENUM GL_UNSIGNED_INT
#endif /* USE_16BIT_INDICES */

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

//#define _GNU_SOURCE

#include "gles2helper/gles2helper.h"
#include "gles2helper/mathelper.h"
#define _DEFINE_GAMETIME
#include "gles2helper/gametime.h"
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <utility>

// Debug settings
#define DEBUG 1
/*#define DEBUG_EXTRA 1*/
// Enable this setting for automatic test runs
// (use with --test to do this)
#define TEST 1 // enables --test

#include "objmaster/Obj.h"
#include "objmaster/ObjMeshObject.h"
#include "objmaster/MaterializedObjModel.h"
#include "objmaster/objmasterlog.h"
#include "objmaster/FileAssetLibrary.h"
#include "objmaster/StbImgTexturePreparationLibrary.h"
#include "objmaster/ext/GlGpuTexturePreparationLibrary.h"

// Only include tests when it is really needed
#ifdef TEST
#include "objmaster/tests/tests.h"
#endif

static int left = 0;
static int right = 0;
static int up = 0;
static int down = 0;

/** The view rotation [x, y, z] */
static GLfloat view_rot[3] = { 20.0, 30.0, 0.0 };
/** The location of the shader uniforms */
static GLuint ModelViewProjectionMatrix_location,
			NormalMatrix_location,
			LightSourcePosition_location,
			MaterialColor_location,
			TextureSampler_location;
/** The projection matrix */
static GLfloat ProjectionMatrix[16];
/** The direction of the directional light for the scene */
static const GLfloat LightSourcePosition[4] = { 5.0, 5.0, 10.0, 1.0};

/** Holds the model of the obj */
static ObjMaster::MaterializedObjModel<ObjMasterExt::GlGpuTexturePreparationLibrary> model;

static void printGlError(std::string where) {
	GLenum err = glGetError();
	if(err != GL_NO_ERROR) {
	OMLOGE((where + " - glError: 0x%x").c_str(), err);
	}
}

/** Setup various vertex and index buffers for the given model to get ready for rendering - call only once! */
static void setup_buffers(GLuint positionLoc, GLuint normalLoc, GLuint texCoordLoc, const ObjMaster::ObjMeshObject &model,
		std::pair<bool, std::pair<GLuint, GLuint>> &indVertBufIdPair) {
	printGlError("Before setup_buffers");
	if(model.inited && (model.vertexCount > 0) && (model.indexCount > 0)) {
		// This little program is really a one-shot renderer so we do not save
		// the object handles. In a bigger application you should handle them properly!
		// Rem.: This is why you call the method at most only once...
		bool notFirstRun = indVertBufIdPair.first; // ensure we do not setup buffers too many times
		GLuint s_vertexPosObject = indVertBufIdPair.second.first;
		GLuint s_indexObject = indVertBufIdPair.second.second;

		if(!notFirstRun) {
			// Generate vertex buffer object
			glGenBuffers(1, &s_vertexPosObject);
			glBindBuffer(GL_ARRAY_BUFFER, s_vertexPosObject );
			glBufferData(GL_ARRAY_BUFFER, model.vertexCount * sizeof(VertexStructure),
					&((*(model.vertexData))[0].x), GL_STATIC_DRAW);

			// Generate index buffer object
			glGenBuffers(1, &s_indexObject);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_indexObject);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					 model.indexCount * sizeof((*(model.indices))[0]),
					 &((*(model.indices))[0]),
					 GL_STATIC_DRAW);
			// Save generated buffers
			indVertBufIdPair.second.first = s_vertexPosObject;
			indVertBufIdPair.second.second = s_indexObject; 
			// Stop doing extra bullshit every frame!
			indVertBufIdPair.first = true;
			//firstRun = false;
		}

		// Bind the vertex buffer object and create two vertex attributes from the bound buffer
		glBindBuffer(GL_ARRAY_BUFFER, s_vertexPosObject);
		// By design, we know that the positions are the first elements in the VertexStructure
		// so we can use zero as the pointer/index in the vertex data!
		glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), 0);
		// Calculate the offset where the normal vector data starts in the vertex data
		// This is much better than writing "3" as this handles changes in the structure...
		auto normalOffset = (&((*(model.vertexData))[0].i) - &((*(model.vertexData))[0].x));
		// Use the calculated offset for getting the pointer to the normals in the vertex data
		glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), (const GLvoid *)(normalOffset * 4));
		// Calculate the offset where the normal vector data starts in the vertex data
		// This is much better than writing "3" as this handles changes in the structure...
		auto texCoordOffset = (&((*(model.vertexData))[0].u) - &((*(model.vertexData))[0].x));
		// Use the calculated offset for getting the pointer to the texcoords in the vertex data
		glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), (const GLvoid *)(texCoordOffset * 4));

		// Enable the vertex attributes as arrays
		glEnableVertexAttribArray(positionLoc);
		glEnableVertexAttribArray(normalLoc);
		glEnableVertexAttribArray(texCoordLoc);

		// Bind the index buffer object we have created
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_indexObject);
		printGlError("after setup_buffers");

#ifdef DEBUG_EXTRA
OMLOGI("Vertex data sent to the GPU:");
if(model.vertexData != nullptr) {
for(int i = 0; i < model.vertexData->size(); ++i) {
	OMLOGI("v(%f, %f, %f) vn(%f, %f, %f) vt(%f, %f)", 
			(*model.vertexData)[i].x,
			(*model.vertexData)[i].y,
			(*model.vertexData)[i].z,
			(*model.vertexData)[i].i,
			(*model.vertexData)[i].j,
			(*model.vertexData)[i].k,
			(*model.vertexData)[i].u,
			(*model.vertexData)[i].v
	);
}
}
OMLOGI("Index data sent to the GPU:");
if(model.indices != nullptr) {
for(int i = 0; i < model.indices->size() / 3; ++i) {
	OMLOGI("f %d %d %d", (*model.indices)[3*i], (*model.indices)[3*i+1], (*model.indices)[3*i+2]);
}
}
#endif
	} else {
		fprintf(stderr, "No available model, vertex data or indices to setup!\n");
		fprintf(stderr, "vertexCount: %d; indexCount: %d; inited: %d\n", model.vertexCount, model.indexCount, model.inited);
	}
}


/** Draw the mesh of the obj file - first version, no material handling */
static void draw_model(const ObjMaster::MaterializedObjMeshObject &model, GLfloat *transform, const GLfloat color[4]){

	GLfloat model_view[16];
	GLfloat normal_matrix[16];
	GLfloat model_view_projection[16];

	/* creating model_view */
	memcpy(model_view, transform, sizeof (model_view));
	// translate and rotate a little bit to "animate" the thing
	//translate(model_view, x, y, 0);
	//rotate(model_view, 2 * M_PI * angle / 360.0, 0, 0, 1);

	/* Create and set the ModelViewProjectionMatrix */
	memcpy(model_view_projection, ProjectionMatrix, sizeof(model_view_projection));
	multiply(model_view_projection, model_view);

	glUniformMatrix4fv(ModelViewProjectionMatrix_location, 1, GL_FALSE,
				model_view_projection);

	/*
	* Create and set the NormalMatrix. It's the inverse transpose of the
	* ModelView matrix.
	*/
	memcpy(normal_matrix, model_view, sizeof (normal_matrix));
	invert(normal_matrix);
	transpose(normal_matrix);
	glUniformMatrix4fv(NormalMatrix_location, 1, GL_FALSE, normal_matrix);

	glUniform4fv(MaterialColor_location, 1, color);

	// Pass the texture sampler uniform
	glUniform1i(TextureSampler_location, 0);

	// TODO: ensure this is the place for this code
	glBindTexture(GL_TEXTURE_2D, model.material.tex_kd.handle);

	printGlError("Before glDrawElements");
	// Some devices does not support 32 bit indices (orange pi, raspberry pi, etc)
	//glDrawElements(GL_TRIANGLES, model.indexCount, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_TRIANGLES, model.indexCount, OM_GL_INDEX_TYPE_ENUM, 0);
	printGlError("After glDrawElements");
}

/**
 * Main drawing routine
 */
static void draw() {
	const static GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
	const static GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
	const static GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };
	GLfloat transform[16];
	identity(transform);

	// Cornflowerblue for having a retro feeling from my XNA years :-)
	glClearColor(0.3921, 0.5843, 0.9294, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Translate and rotate the view */
	translate(transform, 0, 0, -40);
	rotate(transform, 2 * M_PI * view_rot[0] / 360.0, 1, 0, 0);
	rotate(transform, 2 * M_PI * view_rot[1] / 360.0, 0, 1, 0);
	rotate(transform, 2 * M_PI * view_rot[2] / 360.0, 0, 0, 1);

	// Create vector object for holding data buffers
	static std::vector<std::pair<bool, std::pair<GLuint, GLuint>>> indVertBufIdPairs(model.meshes.size());
	// Render the model
	if(model.inited && model.meshes.size() > 0) {
		int i = 0;
		for(auto mesh : model.meshes) {
			// TODO: remove the "color" parameter
			// TODO: This is really suboptimal! The VBOs should
			// not be always overwritten I think but this little
			// example is not performance critical so it is ookay...
			// I mean... the data copy in setup buffers is too much!
			// Setup buffers for rendering the first mesh
			setup_buffers(0, 1, 2, mesh, indVertBufIdPairs[i]); 
			draw_model(mesh, transform, red);
			printGlError("after draw_model");
			++i;
		}
	}
}

int drawUpdate(int hintDraw) {
	// TODO: Update functionality
	static unsigned long long framecunt = 0;
	++framecunt;
	gametime gt = gametime::mainloop_get_current();

	// FPS counter
	static unsigned long long last_fps_shown_at_ms = gt.get_ms();
	unsigned long long now = gt.get_ms();
	unsigned long long diff = now - last_fps_shown_at_ms;
	if(diff >= 1000) {
		last_fps_shown_at_ms = now;
		printf("FPS: %f, framecount: %zd, diff: %zd\n", gt.fps(), framecunt, diff);
		framecunt = 0;
	}

	view_rot[1] += (float)left * gt.delta()*60;
	view_rot[1] -= (float)right* gt.delta()*60;
	view_rot[0] += (float)up * gt.delta()*60;
	view_rot[0] -= (float)down * gt.delta()*60;
	// Hintdraw only comes on key events, but if I keep it pressed nothing comes!
	int moved = left | right | up | down;

	if(hintDraw || moved) {
		draw();
		return 1;
	} else {
		return 0;
	}
}

/**
 * Handles a new window size or exposure.
 *
 * @param width the window width
 * @param height the window height
 */
static void handleViewportReshape(int width, int height) {
	/* Update the projection matrix */
	perspective(ProjectionMatrix, 60.0, width / (float)height, 1.0, 64.0);

	/* Set the viewport */
	glViewport(0, 0, (GLint) width, (GLint) height);
}

/*
static void idle(void) {
	static int frames = 0;
	static double tRot0 = -1.0, tRate0 = -1.0;
	double dt, t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

	if (tRot0 < 0.0)
		tRot0 = t;
	dt = t - tRot0;
	tRot0 = t;

#ifdef TEST_MEMORYPROFILER_ALLOCATIONS_MAP
	// This file is used to test --memoryprofiler linker flag, in which case
	// add some allocation noise.
	static void *allocatedPtr = 0;
	free(allocatedPtr);
	allocatedPtr = malloc(rand() % 10485760);
#endif

	glutPostRedisplay();
	frames++;

	if (tRate0 < 0.0)
		tRate0 = t;
	if (t - tRate0 >= 5.0) {
		GLfloat seconds = t - tRate0;
		GLfloat fps = frames / seconds;
		printf("%d frames in %3.1f seconds = %6.3f FPS\n", frames, seconds,
		fps);
		tRate0 = t;
		frames = 0;
#ifdef LONGTEST
		static runs = 0;
		runs++;
		if (runs == 4) {
	int result = fps;
#ifdef TEST_MEMORYPROFILER_ALLOCATIONS_MAP
	result = 0;
#endif
	REPORT_RESULT();
		}
#endif
	}
}
*/

static const char vertex_shader[] =
"attribute vec3 position;\n"
"attribute vec3 normal;\n"
"attribute vec2 texcoord;\n"
"\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"uniform vec4 LightSourcePosition;\n"
"uniform vec4 MaterialColor;\n"
"\n"
"varying vec4 Color;\n"
"varying vec2 fragTex;\n"
"\n"
"void main(void)\n"
"{\n"
"	 // Transform the normal to eye coordinates\n"
"	 vec3 N = normalize(vec3(NormalMatrix * vec4(normal, 1.0)));\n"
"\n"
"	 // The LightSourcePosition is actually its direction for directional light\n"
"	 vec3 L = normalize(LightSourcePosition.xyz);\n"
"\n"
"	 // Multiply the diffuse value by the vertex color (which is fixed in this case)\n"
"	 // to get the actual color that we will use to draw this vertex with\n"
"	 float diffuse = max(dot(N, L), 0.0);\n"
"	 Color = diffuse * MaterialColor;\n"
"\n"
"	 // Transform the position to clip coordinates\n"
"	 gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);\n"
"	 // Fill the varying textcoord for the fragment shader\n"
"	 fragTex = texcoord;\n"
"}";

static const char fragment_shader[] =
"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"varying vec4 Color;\n"
"varying vec2 fragTex;\n"
"uniform sampler2D texSampler2D;\n"
"\n"
"void main(void)\n"
"{\n"
"	 vec4 texel = texture2D(texSampler2D, fragTex);\n"
//"	 gl_FragColor = vec4(Color.rgb * texel.rgb, texel.a);\n"
//"	 gl_FragColor = Color;\n"
"	 gl_FragColor = texel;\n"
// For testing texture indices
//"	 gl_FragColor = (Color + texel) + vec4(fragTex, 0.5, 1.0);\n"
"}";

static void init_with_model(char* modelFileNameAndPath) {
	GLuint v, f, program;
	const char *p;
	char msg[512];

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	/* Compile the vertex shader */
	p = vertex_shader;
	v = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(v, 1, &p, NULL);
	glCompileShader(v);
	glGetShaderInfoLog(v, sizeof msg, NULL, msg);
	printf("vertex shader info: %s\n", msg);

	/* Compile the fragment shader */
	p = fragment_shader;
	f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(f, 1, &p, NULL);
	glCompileShader(f);
	glGetShaderInfoLog(f, sizeof msg, NULL, msg);
	printf("fragment shader info: %s\n", msg);

	/* Create and link the shader program */
	program = glCreateProgram();
	glAttachShader(program, v);
	glAttachShader(program, f);
	// Attribute location handling is simple enough for this app
	// We just use manual values for the shader variables...
	glBindAttribLocation(program, 0, "position");
	glBindAttribLocation(program, 1, "normal");
	glBindAttribLocation(program, 2, "texcoord");

	glLinkProgram(program);
	glGetProgramInfoLog(program, sizeof msg, NULL, msg);
	printf("info: %s\n", msg);

	/* Enable the shaders */
	glUseProgram(program);

	/* Get the locations of the uniforms so we can access them */
	ModelViewProjectionMatrix_location = glGetUniformLocation(program, "ModelViewProjectionMatrix");
	NormalMatrix_location = glGetUniformLocation(program, "NormalMatrix");
	LightSourcePosition_location = glGetUniformLocation(program, "LightSourcePosition");
	MaterialColor_location = glGetUniformLocation(program, "MaterialColor");
	TextureSampler_location = glGetUniformLocation(program, "texSampler2D");
	/* Set the LightSourcePosition uniform which is constant throught the program */
	glUniform4fv(LightSourcePosition_location, 1, LightSourcePosition);

	// Activate texture unit0 for basic texturing
	glActiveTexture(GL_TEXTURE0);

	// Load models
	// In this example this should never be inited at this point, but wanted to show how to do that check
	// For example in case of android applications with complex app life-cycles it is better to have this...
	if(!model.inited) {
	ObjMaster::Obj obj; 
	if(modelFileNameAndPath == nullptr) {
		obj = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), "./models/", "default.obj");
	} else {
		char* modelPath;
		char* modelFile;
		int modFilePathLen = strlen(modelFileNameAndPath);
		if(modFilePathLen > 0) {
			// Get a pointer pointing to the last character
			char* fileNamePtr = modelFileNameAndPath + modFilePathLen - 1;
			// Find the last '/' character or go to the first character
			while(modelFileNameAndPath < fileNamePtr && *fileNamePtr != '/') {
				--fileNamePtr;
			}
			// We must check if we have found a '/' or there was none!
			if(*fileNamePtr == '/') {
				// Save the data with a string (as we will delete the first letter from it)
				std::string fileNameStr(fileNamePtr+1);
				// If there was, we are on it, so just replace that with a '/' and a 0 byte!
				// this way we will have the path in the original string and the name
				// in this one! This whole thing is necessary only to make loading of
				// associated assets from the place where the file is!
				*fileNamePtr = '/';
				++fileNamePtr;
				*fileNamePtr = 0;
				// We have modified the modelFileNameAndPath to became the path only!!!
				obj = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), modelFileNameAndPath, fileNameStr.c_str());
			} else {
				obj = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), "./", fileNamePtr);
			}
		}
	}
	model = ObjMaster::MaterializedObjModel<ObjMasterExt::GlGpuTexturePreparationLibrary>(obj);
 
	// Load data onto the GPU and setup buffers for rendering
	if(model.inited && model.meshes.size() > 0) {
		// Load textures for the model meshes
		// TODO: Remove unload! This is to test the gl texture lib if unload is possible before load!
		model.unloadAllTextures();
		model.loadAllTextures(ObjMaster::StbImgTexturePreparationLibrary());
	}
	}
}
//
// These are necessary as the gles2helper is not handling init parameters.
static char modelFileNameAndPath[MODEL_PATH_MAX_LEN+1];
static void init() {
	if(modelFileNameAndPath[0] == 0) {
		init_with_model(nullptr);
	} else {
		init_with_model(modelFileNameAndPath);
	}
}


int keyevent(int code, int fields) {
	bool isPress = (bool)(fields & KEYEVENT_ONPRESS);
	bool isRelease = (bool)(fields & KEYEVENT_ONRELEASE);
	bool isSpecial = (bool)(fields & KEYEVENT_IS_SPECIAL);
	printf("c:%d s:%d p:%d r:%d\n",
		code,
		isSpecial,
		isPress,
		isRelease);
	if(isSpecial) {
		// Use special (named) key codes
		// ARROWS
		if (code == GLES2H_LEFT) {
			if(isPress) left = 1;
			if(isRelease) left = 0;
		}
		else if (code == GLES2H_RIGHT) {
			if(isPress) right = 1;
			if(isRelease) right = 0;
		}
		else if (code == GLES2H_UP) {
			if(isPress) up = 1;
			if(isRelease) up = 0;
		}
		else if (code == GLES2H_DOWN) {
			if(isPress) down = 1;
			if(isRelease) down = 0;
		}
	} else {
		// Use ascii codes
		// ESC
		if(code == 27) {
			// EXIT on ESC!
			// App will return (1-1=0)
			return 1;
		}
		// WASD
		if (code == 'a') {
			if(isPress) left = 1;
			if(isRelease) left = 0;
		}
		else if (code == 'd') {
			if(isPress) right = 1;
			if(isRelease) right = 0;
		}
		else if (code == 'w') {
			if(isPress) up = 1;
			if(isRelease) up = 0;
		}
		else if (code == 's') {
			if(isPress) down = 1;
			if(isRelease) down = 0;
		}
		return 0;
	}

	// Keep the app main loop running!
	return 0;
}

int main(int argc, char *argv[]) {
	// Initialize timing
	gametime::init();

#ifdef TEST
	if(argc == 2) {
		if(std::string(argv[1]) == "--test") {
			// Run all tests
			int errorNo = ObjMasterTest::testAll();
			printf("Number of errors on testing: %d\n", errorNo);
			// Just exit program - with the number of errors found as retval!
			return errorNo;
		}
		if(std::string(argv[1]) == "--help") {
			printf("USAGES:\n");
			printf("	 ./showobj my_model.obj	 - show the given obj file.\n");
			printf("	 ./showobj			 - show the default test model (for emscripten build).\n");
			printf("	 ./showobj --help		 - show this help message and quit.\n");
#ifdef TEST
			printf("	 ./showobj --test		 - run all possible unit tests.\n");
#endif
			return 0;	// quit
		}
	}
#endif
	// Otherwise start application!
#ifdef TEST_MEMORYPROFILER_ALLOCATIONS_MAP
	printf("You should see an interactive CPU profiler graph below, and below that an allocation map of the Emscripten main HEAP, with a long blue block of allocated memory.\n");
#endif // PROFILER
	printf("argc:%d\n", argc);

	/* Prepare our initialization function */
	if(argc == 2) {
		// User provided the model to load
		strncpy(modelFileNameAndPath, argv[1], MODEL_PATH_MAX_LEN+1);
	} else {
		// Will indicate that we send a nullptr
		modelFileNameAndPath[0] = 0;
	}

	/* Initialize the window */
	/*
	glutInit(&argc, argv);
	glutInitWindowSize(300, 300);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutCreateWindow("showobj");

	// Set up glut callback functions
	//glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutIdleFunc (idle);
	glutReshapeFunc(handleViewportReshape);
	glutDisplayFunc(draw);
	glutSpecialFunc(handleSpecialGlutEvents);

	glutMainLoop();
	*/

	// TODO: -sEXPORTED_RUNTIME_METHODS=requestFullscreen
	int ret = gles2run(init, drawUpdate, handleViewportReshape, NULL /*idle*/, keyevent /*keyevent*/, 
			"showobj"/*title*/, 300/*w*/, 300 /*h*/, true /*printinfo*/, NULL /*dpyname*/);

	return ret;
}

/* vim: set ts=4 sw=4 tw=0 noet : */
