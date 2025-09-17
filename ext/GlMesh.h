#ifndef OM_GLMESH_H
#define OM_GLMESH_H

/* An opengl mesh prepared to render and updated to the GPU. */
/* Set -DUSE_OES_VAO to have a "vao" */

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
		TextureDataHoldingMaterial *material;
		GLuint vbo = 0;
		GLuint ibo = 0;
		GLsizei indexCount = 0;
		bool uploaded = false;
#ifdef USE_OES_VAO
		GLuint vao = 0; // if using VAOs via OES_vertex_array_object
#endif

	public:
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
				ObjMaster::ObjMeshObject &mesh) noexcept : material(mat) {

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

#ifdef USE_OES_VAO
				// Use VAO if present (WebGL: OES_vertex_array_object)
				glGenVertexArraysOES(1, &vao);
				glBindVertexArrayOES(vao);
#endif

				// Bind the vertex buffer object and create two vertex attributes from the bound buffer
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				// By design, we know that the positions are the first elements in the VertexStructure
				// so we can use zero as the pointer/index in the vertex data!
				glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), 0);
				// Calculate the offset where the normal vector data starts in the vertex data
				// This is much better than writing "3" as this handles changes in the structure...
				auto normalOffset = (&((*(mesh.vertexData))[0].i) - &((*(mesh.vertexData))[0].x));
				// Use the calculated offset for getting the pointer to the normals in the vertex data
				glVertexAttribPointer(normalLoc,
						3, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), (const GLvoid *)(normalOffset * 4));
				// Calculate the offset where the normal vector data starts in the vertex data
				// This is much better than writing "3" as this handles changes in the structure...
				auto texCoordOffset = (&((*(mesh.vertexData))[0].u) - &((*(mesh.vertexData))[0].x));
				// Use the calculated offset for getting the pointer to the texcoords in the vertex data
				glVertexAttribPointer(texCoordLoc,
						2, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), (const GLvoid *)(texCoordOffset * 4));

				// Enable the vertex attributes as arrays
				glEnableVertexAttribArray(positionLoc);
				glEnableVertexAttribArray(normalLoc);
				glEnableVertexAttribArray(texCoordLoc);

				// Bind the index buffer object we have created - this is needed so VAO captures it being bound
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

#ifdef USE_OES_VAO
				glBindVertexArrayOES(0);
#endif
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
	struct GlMeshBinding {
		bool uploaded = false;
		TextureDataHoldingMaterial *material;
		GLuint vbo;
		GLuint ibo;
		GLsizei indexCount;
#ifdef USE_OES_VAO
		GLuint vao = 0; // if using VAOs via OES_vertex_array_object
#endif

		/** Binds a GlMesh so you can draw these */
		inline GlMeshBinding(GlMesh &mesh) noexcept :
				uploaded(mesh.uploaded),
				material(mesh.material),
				vbo(mesh.vbo),
				ibo(mesh.ibo),
				indexCount(mesh.indexCount)
#ifdef USE_OES_VAO
				,vao(mesh.vao)
#endif
				{
#ifdef USE_OES_VAO
			glBindVertexArrayOES(vao);
			glBindVertexArrayOES(0);
#else
			// Without VAO, still very cheap: only bind VBO/IBO if not already bound
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
#endif
		}

		inline ~GlMeshBinding() noexcept {
#ifdef USE_OES_VAO
			glBindVertexArrayOES(0);
#else
			// Without VAO, still very cheap: only unbind VBO/IBO if not already bound
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
		}
	};

	}
#endif /* OM_GLMESH_H */
