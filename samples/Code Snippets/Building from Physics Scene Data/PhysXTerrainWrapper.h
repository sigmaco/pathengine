#ifndef PHYS_X_TERRAIN_WRAPPER_INCLUDED
#define PHYS_X_TERRAIN_WRAPPER_INCLUDED

#include "i_pathengine.h"
#include <vector>

class NxHeightFieldShape;
class cCoordinateMapping;

class cPhysXTerrainWrapper : public iFaceVertexMesh
{
    tSigned32 _stride, _strides;
    std::vector<tSigned32> _pointCoords;
    std::vector<bool> _tessellationFlags;
    std::vector<tSigned32> _holesRemap;

public:

    cPhysXTerrainWrapper(const NxHeightFieldShape& heightFieldShape, const cCoordinateMapping& coordinateMapping, float scale);

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
