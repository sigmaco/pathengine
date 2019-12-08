#include "PhysXTriangleMeshWrapper.h"
#include "CoordinateMapping.h"
#include <NxTriangleMeshShape.h>
#include <NxTriangleMesh.h>
#include <NxTriangleMeshDesc.h>
#include <NxMat34.h>

cPhysXTriangleMeshWrapper::cPhysXTriangleMeshWrapper(const NxTriangleMeshShape& shape, const cCoordinateMapping& coordinateMapping, float scale)
{
    NxMat34 transform = shape.getGlobalPose();
    transform.M *= scale;
    transform.t *= scale;
    const NxTriangleMesh& mesh = shape.getTriangleMesh();
    NxTriangleMeshDesc meshDesc;
    mesh.saveToDesc(meshDesc);
    const char* bytePtr;
    tSigned32 i;
    _pointCoords.reserve(meshDesc.numVertices * 3);
    bytePtr = (const char*)(meshDesc.points);
    for(i = 0; i != static_cast<tSigned32>(meshDesc.numVertices); ++i)
    {
        const NxVec3* data = (const NxVec3*)bytePtr;        
        NxVec3 transformed;
        transform.multiply(*data, transformed);
        _pointCoords.push_back(static_cast<tSigned32>(transformed.x));
        _pointCoords.push_back(static_cast<tSigned32>(transformed.y));
        _pointCoords.push_back(static_cast<tSigned32>(transformed.z));
        bytePtr += meshDesc.pointStrideBytes;
    }
    for(i = 0; i != static_cast<tSigned32>(meshDesc.numVertices); ++i)
    {
        coordinateMapping.applyTo(&_pointCoords[i * 3]);
    }
    _vertexIndices.reserve(meshDesc.numTriangles * 3);
    bytePtr = (const char*)(meshDesc.triangles);
    if(meshDesc.flags & NX_MF_16_BIT_INDICES)
    {
        for(i = 0; i != static_cast<tSigned32>(meshDesc.numTriangles); ++i)
        {
            const NxU16* data = (const NxU16*)bytePtr;
            _vertexIndices.push_back(static_cast<tSigned32>(data[0]));
            _vertexIndices.push_back(static_cast<tSigned32>(data[2])); // reverses winding order
            _vertexIndices.push_back(static_cast<tSigned32>(data[1]));
            bytePtr += meshDesc.triangleStrideBytes;
        }
    }
    else
    {
        for(i = 0; i != static_cast<tSigned32>(meshDesc.numTriangles); ++i)
        {
            const NxU32* data = (const NxU32*)bytePtr;
            _vertexIndices.push_back(static_cast<tSigned32>(data[0]));
            _vertexIndices.push_back(static_cast<tSigned32>(data[2])); // reverses winding order
            _vertexIndices.push_back(static_cast<tSigned32>(data[1]));
            bytePtr += meshDesc.triangleStrideBytes;
        }
    }
}

// iFaceVertex interface
tSigned32
cPhysXTriangleMeshWrapper::faces() const
{
    return static_cast<tSigned32>(_vertexIndices.size() / 3);
}
tSigned32
cPhysXTriangleMeshWrapper::vertices() const
{
    return static_cast<tSigned32>(_pointCoords.size() / 3);
}
tSigned32
cPhysXTriangleMeshWrapper::vertexIndex(tSigned32 face, tSigned32 vertexInFace) const
{
    return _vertexIndices[face * 3 + vertexInFace];
}
tSigned32
cPhysXTriangleMeshWrapper::vertexX(tSigned32 vertex) const
{
    return _pointCoords[vertex * 3 + 0];
}
tSigned32
cPhysXTriangleMeshWrapper::vertexY(tSigned32 vertex) const
{
    return _pointCoords[vertex * 3 + 1];
}
float
cPhysXTriangleMeshWrapper::vertexZ(tSigned32 vertex) const
{
    return static_cast<float>(_pointCoords[vertex * 3 + 2]);
}
tSigned32
cPhysXTriangleMeshWrapper::faceAttribute(tSigned32 face, tSigned32 attributeIndex) const
{
    return -1;
}
