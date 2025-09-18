#ifndef OM_GLMESH_H
#define OM_GLMESH_H

/* An opengl mesh prepared to render and updated to the GPU. */
/* Set -DUSE_VAO to have a "vao" even in ES and webgl 1.x, but USE_FULL_GL enables it if there is no PRE_33_GL (compatibility) */

#ifdef USE_FULL_GL
#ifndef PRE_33_GL
#define USE_VAO // VAO is mandatory in 3.3+ opengl core contexts but not in compatibility!
#endif /* PRE_33_GL */
#endif /* USE_FULL_GL */

#define GL_GLEXT_PROTOTYPES
/* Needs something like:*/
/*#include <GLES2/gl2.h>*/
/*#include <GLES2/gl2platform.h>*/
/*#include <GLES2/gl2ext.h>*/

#include "../objmasterlog.h"
#include "../MaterializedObjMeshObject.h"
#include "../TextureDataHoldingMaterial.h"

namespace ObjMaster {

	/** If you create a binding to a GlMesh, it is prepared for drawing! RAII releases bindings */
	struct GlMeshBinding;

	/** A mesh that is uploaded to the GPU - TODO: need to add destructor to remove from GPU? */
	class GlMesh {
		bool uploaded = false;
		GLsizei indexCount = 0;
		TextureDataHoldingMaterial *material;
		GLuint position_loc;
		GLuint normal_loc;
		GLuint texCoord_loc;
		GLuint vbo = 0;
		GLuint ibo = 0;
#ifdef USE_VAO
		GLuint vao = 0; // if using VAOs via OES_vertex_array_object
#else
		size_t posOffset = 0;
		size_t normalOffset = 0;
		size_t texCoordOffset = 0;
#endif /* USE_VAO */

	public:
		/**
		 * Sets vertex attribute pointers for this mesh data and enables them
		 */
		static inline void setup_vertex_attributes(
				GLuint position_loc,
				GLuint normal_loc,
				GLuint texCoord_loc,
				size_t posOffset,
				size_t normalOffset,
				size_t texCoordOffset) {

			// By design, we know that the positions are the first elements in the VertexStructure
			// so we can use zero as the pointer/index in the vertex data!
			glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), (const GLvoid *) posOffset);
			printGlError("after position attrib pointer");

			// Use the calculated offset for getting the pointer to the normals in the vertex data
			glVertexAttribPointer(normal_loc,
					3, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), (const GLvoid *)(normalOffset * 4));
			printGlError("after normal attrib pointer");

			// Use the calculated offset for getting the pointer to the texcoords in the vertex data
			glVertexAttribPointer(texCoord_loc,
					2, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), (const GLvoid *)(texCoordOffset * 4));
			printGlError("after texcoord attrib pointer");

			// Enable the vertex attributes as arrays
			glEnableVertexAttribArray(position_loc);
			glEnableVertexAttribArray(normal_loc);
			glEnableVertexAttribArray(texCoord_loc);
			printGlError("after enabling vertex attributes");
		}

		/** Empty mesh */
		inline GlMesh() noexcept {}

		/**
		 * Setup various vertex and index buffers for the given mesh to get ready for rendering - call only once!
		 *
		 * @param positionLoc The shader location for position (see shaders and see example)
		 * @param normalLoc The shader location for normals (see shaders and see example)
		 * @param texCoordLoc The shader location for UVs (see shaders and see example)
		 * @param mat The material of the mesh
		 * @param mesh The mesh object reference
		 */
		inline GlMesh(
				GLuint positionLoc,
				GLuint normalLoc,
				GLuint texCoordLoc,
				TextureDataHoldingMaterial *mat,
				ObjMaster::ObjMeshObject &mesh) noexcept
				: material(mat), position_loc(positionLoc), normal_loc(normalLoc), texCoord_loc(texCoordLoc) {

			printGlError("Before setup_buffers");
			if(mesh.inited && (mesh.vertexCount > 0) && (mesh.indexCount > 0)) {
				// Generate vertex buffer object
				glGenBuffers(1, &vbo);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferData(GL_ARRAY_BUFFER, mesh.vertexCount * sizeof(VertexStructure),
						&((*(mesh.vertexData))[0].x), GL_STATIC_DRAW);

				// Generate index buffer object
				glGenBuffers(1, &ibo);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER,
						mesh.indexCount * sizeof((*(mesh.indices))[0]),
						&((*(mesh.indices))[0]),
						GL_STATIC_DRAW);

#ifdef USE_VAO
				// Use VAO if present
				glGenVertexArrays(1, &vao);
				glBindVertexArray(vao);
				// Binding the buffer should be part of VAO state
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
#ifdef USE_FULL_GL
#else
				// Use VAO if present (WebGL: OES_vertex_array_object)
				glGenVertexArraysOES(1, &vao);
				glBindVertexArrayOES(vao);
				// Binding the buffer should be part of VAO state
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
#endif /* USE_FULL_GL */
				auto posOffset = 0;
				// Calculate the offset where the normal vector data starts in the vertex data
				// This is much better than writing "3" as this handles changes in the structure...
				auto normalOffset = (&((*(mesh.vertexData))[0].i) - &((*(mesh.vertexData))[0].x));
				// Calculate the offset where the normal vector data starts in the vertex data
				// This is much better than writing "3" as this handles changes in the structure...
				auto texCoordOffset = (&((*(mesh.vertexData))[0].u) - &((*(mesh.vertexData))[0].x));
#else
				posOffset = 0;
				// Calculate the offset where the normal vector data starts in the vertex data
				// This is much better than writing "3" as this handles changes in the structure...
				normalOffset = (&((*(mesh.vertexData))[0].i) - &((*(mesh.vertexData))[0].x));
				// Calculate the offset where the normal vector data starts in the vertex data
				// This is much better than writing "3" as this handles changes in the structure...
				texCoordOffset = (&((*(mesh.vertexData))[0].u) - &((*(mesh.vertexData))[0].x));
#endif /* USE_VAO */
				printGlError("after buffer creation and binding");

				// This state will go into the VAO when using one, otherwise because
				// VBO and IBO does NOT store this, need to happen every draw of us.
				setup_vertex_attributes(
						position_loc,
						normal_loc,
						texCoord_loc,
						posOffset,
						normalOffset,
						texCoordOffset);

				// Bind the index buffer object we have created - this is needed so VAO captures it being bound
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
				printGlError("after binding ibo in setup");

#ifdef USE_VAO
#ifdef USE_FULL_GL
				glBindVertexArray(0);
#else
				glBindVertexArrayOES(0);
#endif /* USE_FULL_GL */
#endif /* USE_VAO */
				// Unbind to avoid accidental changes
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				/*if (!vao)*/ glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				indexCount = mesh.indexCount;
				uploaded = true;

				printGlError("after setup_buffers");

#ifdef DEBUG_EXTRA
				OMLOGI("Vertex data sent to the GPU:");
				if(mesh.vertexData != nullptr) {
					for(int i = 0; i < mesh.vertexData->size(); ++i) {
						OMLOGI("v(%f, %f, %f) vn(%f, %f, %f) vt(%f, %f)", 
								(*mesh.vertexData)[i].x,
								(*mesh.vertexData)[i].y,
								(*mesh.vertexData)[i].z,
								(*mesh.vertexData)[i].i,
								(*mesh.vertexData)[i].j,
								(*mesh.vertexData)[i].k,
								(*mesh.vertexData)[i].u,
								(*mesh.vertexData)[i].v
							  );
					}
				}
				OMLOGI("Index data sent to the GPU:");
				if(mesh.indices != nullptr) {
					for(int i = 0; i < mesh.indices->size() / 3; ++i) {
						OMLOGI("f %d %d %d", (*mesh.indices)[3*i], (*mesh.indices)[3*i+1], (*mesh.indices)[3*i+2]);
					}
				}
#endif
			} else {
				OMLOGE("No available mesh, vertex data or indices to setup!\n");
				OMLOGE("vertexCount: %d; indexCount: %d; inited: %d\n", mesh.vertexCount, mesh.indexCount, mesh.inited);
			}
		}

		friend class GlMeshBinding;
	};

	/** If you create a binding to a GlMesh, it is prepared for drawing! RAII releases bindings */
	class GlMeshBinding {
	private:
		GLuint vbo = 0;
		GLuint ibo = 0;
#ifdef USE_VAO
		GLuint vao = 0; // if using VAOs via OES_vertex_array_object
#else
		size_t posOffset = 0;
		size_t normalOffset = 0;
		size_t texCoordOffset = 0;
#endif
	public:
		bool uploaded = false;
		GLsizei indexCount = 0;
		GLuint position_loc;
		GLuint normal_loc;
		GLuint texCoord_loc;
		TextureDataHoldingMaterial *material;

		/** Binds a GlMesh so you can draw these */
		inline GlMeshBinding(GlMesh &mesh) noexcept :
				vbo(mesh.vbo),
				ibo(mesh.ibo),
#ifdef USE_VAO
				vao(mesh.vao),
#else
				posOffset(mesh.posOffset),
				normalOffset(mesh.normalOffset),
				texCoordOffset(mesh.texCoordOffset),
#endif
				uploaded(mesh.uploaded),
				indexCount(mesh.indexCount),
				position_loc(mesh.position_loc),
				normal_loc(mesh.normal_loc),
				texCoord_loc(mesh.texCoord_loc),
				material(mesh.material) {
#ifdef USE_VAO
#ifdef USE_FULL_GL
			glBindVertexArray(vao);
#else
			glBindVertexArrayOES(vao);
#endif /* USE_FULL_GL */
#else
			// With no VAO support its more pricy
			// because: 2 buffer bindings + attribute setup both needed!
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

			// This state will go into the VAO when using one, otherwise because
			// VBO and IBO does NOT store this, need to happen every draw of us.
			GlMesh::setup_vertex_attributes(
					position_loc,
					normal_loc,
					texCoord_loc,
					posOffset,
					normalOffset,
					texCoordOffset);
#endif /* USE_VAO */
		}

		inline ~GlMeshBinding() noexcept {
#ifdef USE_VAO
#ifdef USE_FULL_GL
			glBindVertexArray(0);
#else
			glBindVertexArrayOES(0);
#endif /* USE_FULL_GL */
#else
			// Without VAO, still very cheap: only unbind VBO/IBO if not already bound
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif /* USE_VAO */
		}
	};
}
#endif /* OM_GLMESH_H */
