//
// Created by rthier on 2016.04.12..
//

#ifndef OBJMASTER_OBJMESHOBJECT_H
#define OBJMASTER_OBJMESHOBJECT_H

#include "Obj.h"
#include "FaceElement.h"
#include "VertexStructure.h"
#include <memory>
#include <vector>
#include <stdint.h>

namespace ObjMaster {
    /**
     * A 3d mesh out of a *.OBJ file. This can be used to show the object in a scene as it has proper
     * buffers one can use with most CG rendering methods (like OpenGL).
     */
    class ObjMeshObject {
    public:
        /**
         * Says if the mesh has been initialized or not.
         * This is useful when using non-pointer object and you cannot see
         * if it is "nullptr" in that case but need extremal element to decide caching etc.
         */
        bool inited = false;

	// These are pointers as they are useful for optimized shared vectors in bigger models!
	// In other case we use them to point to our locally created vectors that we create with new!
	// If someone is trying to use the library in that way that each object is one mesh that still
	// still needs to be as fast as it can be and there is a slight overhead in shared_ptr too so
	// it is better to do a little housekeeping than to make the implementation easier for me.
	/** If true the mesh object is owning the pointer, otherwise the other object that logically contains the (multiple) meshes */
	bool ownsVertexData = false; // ownership flag
	/** If true the mesh object is owning the pointer, otherwise the other object that logically contains the (multiple) meshes */
	bool ownsIndices = false; // ownership flag
        // Render-ready data with inlined vertex attributes. Inlining is usually better for GPU
        // cache hitting.
	/**
	 * Pointer to the vector of vertex data.
	 *  - Beware as this can be a shared vector in case of the mesh is being part of a complex model!
	 *  - Because of this, one should use baseVertexLocation to find the start vertex and vertexCount
	 *    for the mesh-specific size! (the vector would return the whole model vertices count otherwise)
	 */
	std::vector<VertexStructure> *vertexData;
	/**
	 * Pointer to the vector of index data.
	 *  - Beware as this can be a shared vector in case of the mesh is being part of a complex model!
	 *  - Because of this, one should use startIndexLocation to find the start index and indexCount
	 *    for the mesh-specific size! (the vector would return the whole model index-count otherwise)
	 */
	std::vector<uint32_t> *indices;

	// Helper variables to aid optimized one-vbo rendering of multi-mesh models
	/**
	 * MULTIMESH: The location of the first index for the GPU to read from the index buffer.
	 * When a model contains multiple meshes, it is much better to put all the mesh data into
	 * the one and same vertex and index buffers (given that the object is not too big). In
	 * that case, we of course need to know where the data would start in a big buffer like
	 * that. Because the model loading process has this information, we provide it here so
	 * using the vertex data for this mesh and the other meshes, one can construct the big
	 * continous array if that is what you want!
	 *
	 * In non-multimesh cases this will be just 0
	 */
	unsigned int startIndexLocation;
	/**
	 * MULTIMESH: A value added to each index before reading a vertex from the vertex buffer.
	 * When a model contains multiple meshes, it is much better to put all the mesh data into
	 * the one and same vertex and index buffers (given that the object is not too big). In
	 * that case, we of course need to know where the data would start in a big buffer like
	 * that. Because the model loading process has this information, we provide it here so
	 * using the vertex data for this mesh and the other meshes, one can construct the big
	 * continous array if that is what you want!
	 *
	 * In non-multimesh cases this will be just 0
	 */
	unsigned int baseVertexLocation;
	/** The number of indices (the per-mesh value - not the indices.size() which might be bigger because of sharing!!!) */
	unsigned int indexCount;
	/** The number of vertices (the per-mesh value - not the vertexData.size() which might be bigger because of sharing!!!) */
	unsigned int vertexCount;
	/** The biggest index value that belongs to this mesh */
	uint32_t lastIndex;
        // Empty constructor
        ObjMeshObject() {};

        /** Create an obj mesh object using all faces available in the given Obj */
        ObjMeshObject(const Obj& obj);

        /**
         * Create and obj mesh object using the explicitly given faces from the given Obj.
         * The parameter variables should be in synch with each other and refer to data from the
         * same *.obj file / same Obj data.
         */
        ObjMeshObject(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount);

        /**
         * Create and obj mesh object using the explicitly given faces from the given Obj.
         * The parameter variables should be in synch with each other and refer to data from the
         * same *.obj file / same Obj data. The index and vertex data will be collected in the
	 * given vectors instead of creating new vectors here in the constructor for this
	 * purpose! The passed pointers are not owned by the mesh and freeing them is the job
	 * of the caller. Also this kind of construction creates a tighter coupling of the 
	 * life-cycles of the vectors provided by the caller and the created object! The reason
	 * why this is useful is that using the mesh for creating parts of a big model can be done
	 * in a much more resource effective way. In case any of the last two parameters are null,
	 * the constructor creates the vector for them. The lastIndexBase parameter is useful for
	 * defining from which point the indices should start. Basically this should be max(indexVector)
	 * if the indexVector is a non-null and non-empty pointer and zero otherwise!!!
         */
        ObjMeshObject(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount, std::vector<VertexStructure> *vertexVector, std::vector<uint32_t> *indexVector, uint32_t lastIndexBase);

	/**
	 * The copy ctor - necessary because of the possible pointer sharing stuff!
	 * In case of non-shared vectors we copy, in case of shared vectors we copy only the pointer!
	 */
	ObjMeshObject(const ObjMeshObject &other);

	/**
	 * The copy assignment op - necessary because of the possible pointer sharing stuff!
	 * In case of non-shared vectors we copy, in case of shared vectors we copy only the pointer!
	 */
	ObjMeshObject& operator=(const ObjMeshObject& other);

	/**
	 * The move assignment op - necessary because of the possible pointer sharing stuff!
	 * In case of non-shared vectors we move, in case of shared vectors we move only the pointer!
	 */
	ObjMeshObject(ObjMeshObject &&other);

	/**
	 * The move assignment op - necessary because of the possible pointer sharing stuff!
	 * In case of non-shared vectors we move, in case of shared vectors we move only the pointer!
	 */
	ObjMeshObject& operator=(ObjMeshObject&& other);

	// The destructor needs to delete the pointed vectors only in case we own them!
	~ObjMeshObject() {
		if (ownsVertexData) { delete vertexData; }
		if (ownsIndices) { delete indices; }
	}
    private:
        void creationHelper(const Obj& obj, const FaceElement *meshFaces, int meshFaceCount, std::vector<VertexStructure> *vertexVector, std::vector<uint32_t> *indexVector, uint32_t lastIndexBase);
	void copyHelper(const ObjMeshObject &other);
	void moveHelper(ObjMeshObject &&other);
    };
}
#endif
