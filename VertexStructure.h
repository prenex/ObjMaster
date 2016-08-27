//
// Created by rthier on 2016.04.20..
//

#ifndef NFTSIMPLEPROJ_VERTEXSTRUCTURE_H
#define NFTSIMPLEPROJ_VERTEXSTRUCTURE_H

/**
 * This defines the data structure for the vertices when prepared for rendering. An index in the
 * index buffer refer to one of these. Because of better GPU cache hitting, these data should be
 * in an array that contains the strides for the various attributes. To obtain most compatibility
 * You should refer to logically corresponding parts of the structure by referring to the starting
 * element of it. For example like this:
 * glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), &modelVertices[0].x);
 * glVertexAttribPointer(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexStructure), &modelVertices[0].u);
 * The vertex structure MUST start with the position attribute x,y,z - this is the reference point!
 *
 * Using the above approach is surely one that works in the long term as the corresponding elements
 * will be always after each other in this structure in the future by definition. So even if the
 * structure grows by the ObjMaster library starting to handle more data - your code will not break!
 *
 * Bigger changes (like 2x 3x bigger structure sizes and such) will be handled by creating new
 * structures and functions in parallel to the already existing, so performance will stay mostly
 * constant too in-between various versions of the ObjMaster.
 */
struct VertexStructure {
    // position
    float x, y, z;
    // normal
    float i, j, k;
    // texture1 uv
    float u, v;
};

#endif //NFTSIMPLEPROJ_VERTEXSTRUCTURE_H
