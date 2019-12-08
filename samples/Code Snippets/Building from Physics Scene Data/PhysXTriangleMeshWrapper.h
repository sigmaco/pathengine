#ifndef PHYS_X_TRIANGLE_MESH_WRAPPER_INCLUDED

#include "i_pathengine.h"
#include <vector>

class NxTriangleMeshShape;
class cCoordinateMapping;

class cPhysXTriangleMeshWrapper : public iFaceVertexMesh
{
    std::vector<tSigned32> _pointCoords;
    std::vector<tSigned32> _vertexIndices;

public:

    cPhysXTriangleMeshWrapper(const NxTriangleMeshShape& shape, const cCoordinateMapping& coordinateMapping, float scale);

// iFaceVertex interface

    tSigned32 faces() const;
    tSigned32 vertices() const;
    tSigned32 vertexIndex(tSigned32 face, tSigned32 vertexInFace) const;
    tSigned32 vertexX(tSigned32 vertex) const;
    tSigned32 vertexY(tSigned32 vertex) const;
    float vertexZ(tSigned32 vertex) const;
    tSigned32 faceAttribute(tSigned32 face, tSigned32 attributeIndex) const;
};

#endif
