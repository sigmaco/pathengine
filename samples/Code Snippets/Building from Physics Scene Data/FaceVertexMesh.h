#ifndef FACE_VERTEX_MESH_INCLUDED

#include "i_pathengine.h"
#include <vector>

class cCoordinateMapping;

class cFaceVertexMesh : public iFaceVertexMesh
{
    std::vector<tSigned32> _vertexCoords;
    std::vector<tSigned32> _vertexIndices;

public:

    //void addTri(const std::vector<tSigned32>& triVertexCoords);
    void addTri(const tSigned32* triVertexCoords);

    void applyCoordinateMapping(const cCoordinateMapping& mapping);

// iFaceVertexMesh interface

    tSigned32 faces() const;
    tSigned32 vertices() const;
    tSigned32 vertexIndex(tSigned32 face, tSigned32 vertexInFace) const;
    tSigned32 vertexX(tSigned32 vertex) const;
    tSigned32 vertexY(tSigned32 vertex) const;
    float vertexZ(tSigned32 vertex) const;
    tSigned32 faceAttribute(tSigned32 face, tSigned32 attributeIndex) const;
};

#endif
