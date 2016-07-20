/*
 * ObjMaster/examples/showobj
 *  
 * Example obj model renderer using the library.
 * 
 * This example uses a subset of GLESv2 and EGL
 * in a way that it can be compiled both with
 * gcc and emscripten (to provide webgl html)
 * 
 * Prerequisite:
 *   freeglut3, freeglut3-dev (both)
 *   libegl1-mesa-dev, libgles2-mesa-dev (EGL/GLES2)
 *   emsdk (JS/WEBGL - full toolchain: nodejs, LLVM, etc.)
 * Compilations:
 *   g++ showobj.cpp -o showobj.out -lglut -lGLESv2 -lm -std=c++14
 *   em++ showobj.cpp -o showobj.html -std=c++14
 * TODO: Usage:
 *   ./showobj <model>     - shows the given model
 *   ./showobj             - shows default.obj (as the html build)
 *   palemoon showobj.html - open webgl build to show embedded default.obj
 */

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

//#define _GNU_SOURCE

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <unistd.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <Glut/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#define DEBUG 1
#define DEBUG_EXTRA

#include "objmaster/Obj.h"
#include "objmaster/ObjMeshObject.h"
#include "objmaster/objmasterlog.h"
#include "objmaster/FileAssetLibrary.h"

#ifndef HAVE_BUILTIN_SINCOS
#define sincos _sincos
static void sincos (double a, double *s, double *c) {
  *s = sin (a);
  *c = cos (a);
}
#endif

/** The view rotation [x, y, z] */
static GLfloat view_rot[3] = { 20.0, 30.0, 0.0 };
/** The location of the shader uniforms */
static GLuint ModelViewProjectionMatrix_location,
              NormalMatrix_location,
              LightSourcePosition_location,
              MaterialColor_location;
/** The projection matrix */
static GLfloat ProjectionMatrix[16];
/** The direction of the directional light for the scene */
static const GLfloat LightSourcePosition[4] = { 5.0, 5.0, 10.0, 1.0};

/** Holds the model of the obj */
static ObjMaster::ObjMeshObject objModel;

/**
 * Multiplies two 4x4 matrices.
 *
 * The result is stored in matrix m.
 *
 * @param m the first matrix to multiply
 * @param n the second matrix to multiply
 */
static void multiply(GLfloat *m, const GLfloat *n) {
   GLfloat tmp[16];
   const GLfloat *row, *column;
   div_t d;
   int i, j;

   for (i = 0; i < 16; i++) {
      tmp[i] = 0;
      d = div(i, 4);
      row = n + d.quot * 4;
      column = m + d.rem;
      for (j = 0; j < 4; j++)
         tmp[i] += row[j] * column[j * 4];
   }
   memcpy(m, &tmp, sizeof tmp);
}

/**
 * Rotates a 4x4 matrix.
 *
 * @param[in,out] m the matrix to rotate
 * @param angle the angle to rotate
 * @param x the x component of the direction to rotate to
 * @param y the y component of the direction to rotate to
 * @param z the z component of the direction to rotate to
 */
static void rotate(GLfloat *m, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
   double sd, cd;
   sincos(angle, &sd, &cd);
   float s = (float)sd;
   float c = (float)cd;
   GLfloat r[16] = {
      x * x * (1 - c) + c,     y * x * (1 - c) + z * s, x * z * (1 - c) - y * s, 0,
      x * y * (1 - c) - z * s, y * y * (1 - c) + c,     y * z * (1 - c) + x * s, 0,
      x * z * (1 - c) + y * s, y * z * (1 - c) - x * s, z * z * (1 - c) + c,     0,
      0, 0, 0, 1
   };

   multiply(m, r);
}


/**
 * Translates a 4x4 matrix.
 *
 * @param[in,out] m the matrix to translate
 * @param x the x component of the direction to translate to
 * @param y the y component of the direction to translate to
 * @param z the z component of the direction to translate to
 */
static void translate(GLfloat *m, GLfloat x, GLfloat y, GLfloat z) {
   GLfloat t[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  x, y, z, 1 };

   multiply(m, t);
}

/**
 * Creates an identity 4x4 matrix.
 *
 * @param m the matrix make an identity matrix
 */
static void identity(GLfloat *m) {
   GLfloat t[16] = {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0,
   };

   memcpy(m, t, sizeof(t));
}

/**
 * Transposes a 4x4 matrix.
 *
 * @param m the matrix to transpose
 */
static void transpose(GLfloat *m) {
   GLfloat t[16] = {
      m[0], m[4], m[8],  m[12],
      m[1], m[5], m[9],  m[13],
      m[2], m[6], m[10], m[14],
      m[3], m[7], m[11], m[15]};

   memcpy(m, t, sizeof(t));
}

/**
 * Inverts a 4x4 matrix.
 *
 * This function can currently handle only pure translation-rotation matrices.
 * Read http://www.gamedev.net/community/forums/topic.asp?topic_id=425118
 * for an explanation.
 */
static void invert(GLfloat *m) {
   GLfloat t[16];
   identity(t);

   // Extract and invert the translation part 't'. The inverse of a
   // translation matrix can be calculated by negating the translation
   // coordinates.
   t[12] = -m[12]; t[13] = -m[13]; t[14] = -m[14];

   // Invert the rotation part 'r'. The inverse of a rotation matrix is
   // equal to its transpose.
   m[12] = m[13] = m[14] = 0;
   transpose(m);

   // inv(m) = inv(r) * inv(t)
   multiply(m, t);
}

/**
 * Calculate a perspective projection transformation.
 *
 * @param m the matrix to save the transformation in
 * @param fovy the field of view in the y direction
 * @param aspect the view aspect ratio
 * @param zNear the near clipping plane
 * @param zFar the far clipping plane
 */
void perspective(GLfloat *m, GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
   GLfloat tmp[16];
   identity(tmp);

   double sine, cosine, cotangent, deltaZ;
   GLfloat radians = fovy / 2 * M_PI / 180;

   deltaZ = zFar - zNear;
   sincos(radians, &sine, &cosine);

   if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
      return;

   cotangent = cosine / sine;

   tmp[0] = cotangent / aspect;
   tmp[5] = cotangent;
   tmp[10] = -(zFar + zNear) / deltaZ;
   tmp[11] = -1;
   tmp[14] = -2 * zNear * zFar / deltaZ;
   tmp[15] = 0;

   memcpy(m, tmp, sizeof(tmp));
}

/** Draw the mesh of the obj file - first version, no material handling */
static void draw_model(const ObjMaster::ObjMeshObject &model, GLfloat *transform, const GLfloat color[4]){
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

   glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_INT, 0);
}

static void printGlError(std::string where) {
   GLenum err = glGetError();
   if(err != GL_NO_ERROR) {
   	OMLOGE((where + " - glError: 0x%x").c_str(), err);
   }
}

/** Setup the test buffers */
static void setup_tri(GLfloat* s_vertices, GLuint* s_indices, int vertDataSize, int vdCount, int iCount) {
// 	static const GLfloat s_vertices[] = {
// 		-1, -1, 0,	// vertx
// 		1, 0, 0,	// color
// 		1, -1 ,	0,	// vertx
// 		0, 1, 0,	// color
// 		0,  1 , 0,
// 		0, 0, 1,
// 		1,  0 , 0,
// 		1, 1, 1
// 	};
// 	static const GLuint s_indices[6] = {
// 		0, 1, 2,
// 		0, 1, 3
// 	};

	GLuint	s_vertexPosObject, s_indexObject;

	// Gen VBO
	glGenBuffers(1, &s_vertexPosObject);
	glBindBuffer(GL_ARRAY_BUFFER, s_vertexPosObject );
	glBufferData(GL_ARRAY_BUFFER, vertDataSize * vdCount, s_vertices, GL_STATIC_DRAW );

	// Gen IBO
	glGenBuffers(1, &s_indexObject);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, s_indexObject );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, iCount * 4, s_indices, GL_STATIC_DRAW );

	// Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, s_vertexPosObject);
	// Create attric pointers
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, vertDataSize, 0 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, vertDataSize, (const GLvoid *)(3 * 4) );
	// Enable attrib pointers
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	// Bind IBO
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, s_indexObject );
}

/** Draws the familiar test triangle if setup properly */
static void draw_tri() {
   GLfloat model_view_projection[16];
   identity(model_view_projection);
   glUniformMatrix4fv(ModelViewProjectionMatrix_location, 1, GL_FALSE,
                      model_view_projection);
//   GLfloat mat[16], rot[16], scale[16];
//   /* Set modelview/projection matrix */
//   make_z_rot_matrix(view_rotx, rot);
//   make_scale_matrix(0.5, 0.5, 0.5, scale);
//   mul_matrix(mat, rot, scale);
//   glUniformMatrix4fv(u_matrix, 1, GL_FALSE, mat);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   {
//       glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
//       glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, colors);
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);

      //glDrawArrays(GL_TRIANGLES, 0, 3);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);
   }

   glutSwapBuffers();
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

   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* Translate and rotate the view */
   translate(transform, 0, 0, -40);
   rotate(transform, 2 * M_PI * view_rot[0] / 360.0, 1, 0, 0);
   rotate(transform, 2 * M_PI * view_rot[1] / 360.0, 0, 1, 0);
   rotate(transform, 2 * M_PI * view_rot[2] / 360.0, 0, 0, 1);

   // Render the model mesh
   draw_model(objModel, transform, red);
   printGlError("after draw_model");

   // Render the scene
   glutSwapBuffers();

#ifdef LONGTEST
   glutPostRedisplay(); // check for issues with not throttling calls
#endif
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

/**
 * Handles special glut events.
 *
 * @param special the event to handle.
 */
static void handleSpecialGlutEvents(int special, int crap, int morecrap)
{
   switch (special) {
      case GLUT_KEY_LEFT:
         view_rot[1] += 5.0;
         break;
      case GLUT_KEY_RIGHT:
         view_rot[1] -= 5.0;
         break;
      case GLUT_KEY_UP:
         view_rot[0] += 5.0;
         break;
      case GLUT_KEY_DOWN:
         view_rot[0] -= 5.0;
         break;
      case GLUT_KEY_F11:
         glutFullScreen();
         break;
   }
}

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

// static const char *fragment_shader=
// "#ifdef GL_ES\n"
// "precision mediump float;\n"
// "#endif\n"
// "varying vec4 v_color;\n"
// "void main() {\n"
// "   gl_FragColor = v_color;\n"
// "}\n";
// static const char *vertex_shader=
// "uniform mat4 ModelViewProjectionMatrix;\n"
// "attribute vec4 pos;\n"
// "attribute vec4 color;\n"
// "varying vec4 v_color;\n"
// "void main() {\n"
// "   gl_Position = pos;\n"
// "   v_color = color;\n"
// "}\n";

static const char vertex_shader[] =
"attribute vec3 position;\n"
"attribute vec3 normal;\n"
"\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"uniform vec4 LightSourcePosition;\n"
"uniform vec4 MaterialColor;\n"
"\n"
"varying vec4 Color;\n"
"\n"
"void main(void)\n"
"{\n"
"    // Transform the normal to eye coordinates\n"
"    vec3 N = normalize(vec3(NormalMatrix * vec4(normal, 1.0)));\n"
"\n"
"    // The LightSourcePosition is actually its direction for directional light\n"
"    vec3 L = normalize(LightSourcePosition.xyz);\n"
"\n"
"    // Multiply the diffuse value by the vertex color (which is fixed in this case)\n"
"    // to get the actual color that we will use to draw this vertex with\n"
"    float diffuse = max(dot(N, L), 0.0);\n"
"    Color = diffuse * MaterialColor;\n"
"\n"
"    // Transform the position to clip coordinates\n"
"    gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);\n"
"}";

static const char fragment_shader[] =
"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"varying vec4 Color;\n"
"\n"
"void main(void)\n"
"{\n"
"    gl_FragColor = Color;\n"
"}";

/** Setup various vertex and index buffers for the given model to get ready for rendering - call only once! */
static void setup_buffers(GLuint positionLoc, GLuint normalLoc, const ObjMaster::ObjMeshObject &model) {
	if(model.inited && (model.vertexData.size() > 0) && (model.indices.size() > 0)) {
   		printGlError("before setup_buffers");
		// This little program is really a one-shot renderer so we do not save
		// the object handles. In a bigger application you should handle them properly!
		// Rem.: This is why you call the method at most only once...
		GLuint s_vertexPosObject, s_indexObject;

		// Generate vertex buffer object
		glGenBuffers(1, &s_vertexPosObject);
		glBindBuffer(GL_ARRAY_BUFFER, s_vertexPosObject );
		glBufferData(GL_ARRAY_BUFFER, model.vertexData.size() * sizeof(VertexStructure), &(model.vertexData[0].x), GL_STATIC_DRAW);

		// Generate index buffer object
		glGenBuffers(1, &s_indexObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_indexObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(model.indices[0]), &(model.indices[0]), GL_STATIC_DRAW);

		// Bind the vertex buffer object and create two vertex attributes from the bound buffer
		glBindBuffer(GL_ARRAY_BUFFER, s_vertexPosObject);
		// By design, we know that the positions are the first elements in the VertexStructure
		// so we can use zero as the pointer/index in the vertex data!
		glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), 0);
		// Calculate the offset where the normal vector data starts in the vertex data
		// This is much better than writing "3" as this handles changes in the structure...
		auto normalOffset = (&(objModel.vertexData[0].i) - &(objModel.vertexData[0].x));
		// Use the calculated offset for getting the pointer to the normals in the vertex data
		glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), (const GLvoid *)(normalOffset * 4));

		// Enable the vertex attributes as arrays
		glEnableVertexAttribArray(positionLoc);
		glEnableVertexAttribArray(normalLoc);

		// Bind the index buffer object we have created
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_indexObject);
   		printGlError("after setup_buffers");

#ifdef DEBUG_EXTRA
OMLOGE("Vertex data sent to the GPU:");
for(int i = 0; i < model.vertexData.size(); ++i) {
	OMLOGE("v(%f, %f, %f) vn(%f, %f, %f) vt(%f, %f)", 
			model.vertexData[i].x,
			model.vertexData[i].y,
			model.vertexData[i].z,
			model.vertexData[i].i,
			model.vertexData[i].j,
			model.vertexData[i].k,
			model.vertexData[i].u,
			model.vertexData[i].v
	);
}
OMLOGE("Index data sent to the GPU:");
for(int i = 0; i < model.indices.size() / 3; ++i) {
	OMLOGE("f %d %d %d", model.indices[3*i], model.indices[3*i+1], model.indices[3*i+2]);
}
#endif
	} else {
		fprintf(stderr, "No available model, vertex data or indices to setup!");
	}
}

static void init(void) {
   GLuint v, f, program;
   const char *p;
   char msg[512];

   //glEnable(GL_CULL_FACE);
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
//   glBindAttribLocation(program, 0, "pos");
//   glBindAttribLocation(program, 1, "color");

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
   /* Set the LightSourcePosition uniform which is constant throught the program */
   glUniform4fv(LightSourcePosition_location, 1, LightSourcePosition);

   // Load models
   // In this example this should never be inited at this point, but wanted to show how to do that check
   // For example in case of android applications with complex app life-cycles it is better to have this...
   if(!objModel.inited) {
	ObjMaster::Obj obj = ObjMaster::Obj(ObjMaster::FileAssetLibrary(), "./models/", "red_clothes_lady.obj");
	objModel = ObjMaster::ObjMeshObject(obj);
 
 	// Load data onto the GPU and setup buffers for rendering
 	setup_buffers(0, 1, objModel);
//	setup_tri(&(objModel.vertexData[0].x), &(objModel.indices[0]), (int)sizeof(VertexStructure), objModel.vertexData.size(), objModel.indices.size());
    }
}

int main(int argc, char *argv[]) {
#ifdef TEST_MEMORYPROFILER_ALLOCATIONS_MAP
   printf("You should see an interactive CPU profiler graph below, and below that an allocation map of the Emscripten main HEAP, with a long blue block of allocated memory.\n");
#endif
   /* Initialize the window */
   glutInit(&argc, argv);
   glutInitWindowSize(300, 300);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

   glutCreateWindow("showobj");

   /* Set up glut callback functions */
   glutIdleFunc (idle);
   glutReshapeFunc(handleViewportReshape);
   //glutDisplayFunc(draw_tri);
   glutDisplayFunc(draw);
   glutSpecialFunc(handleSpecialGlutEvents);

   /* Do our initialization */
   init();

   glutMainLoop();

   return 0;
}
