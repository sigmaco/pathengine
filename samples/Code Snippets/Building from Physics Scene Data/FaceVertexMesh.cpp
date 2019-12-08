#include "FaceVertexMesh.h"
#include "CoordinateMapping.h"
#include <assert.h>

//void
//cFaceVertexMesh::addTri(const std::vector<tSigned32>& triVertexCoords)
//{
//    assert(triVertexCoords.size() == 9);
//    tSigned32 firstIndex = static_cast<tSigned32>(_vertexCoords.size() / 3);
//    tSigned32 i;
//    for(i = 0; i != 9; ++i)
//    {
//        _vertexCoords.push_back(triVertexCoords[i]);
//    }
//    _vertexIndices.push_back(firstIndex);
//    _vertexIndices.push_back(firstIndex + 1);
//    _vertexIndices.push_back(firstIndex + 2);
//}
void
cFaceVertexMesh::addTri(const tSigned32* triVertexCoords)
{
    //assert(triVertexCoords.size() == 9);
    tSigned32 firstIndex = static_cast<tSigned32>(_vertexCoords.size() / 3);
    tSigned32 i;
    for(i = 0; i != 9; ++i)
    {
        _vertexCoords.push_back(triVertexCoords[i]);
    }
    _vertexIndices.push_back(firstIndex);
    _vertexIndices.push_back(firstIndex + 1);
    _vertexIndices.push_back(firstIndex + 2);
}
    
void
cFaceVertexMesh::applyCoordinateMapping(const cCoordinateMapping& mapping)
{
    for(tSigned32 i = 0; i != vertices(); ++i)
    {
        mapping.applyTo(&_vertexCoords[i * 3]);
    }
}

// iFaceVertexMesh interface

tSigned32
cFaceVertexMesh::faces() const
{
    return static_cast<tSigned32>(_vertexIndices.size() / 3);
}
tSigned32
cFaceVertexMesh::vertices() const
{
    return static_cast<tSigned32>(_vertexCoords.size() / 3);
}
tSigned32
cFaceVertexMesh::vertexIndex(tSigned32 face, tSigned32 vertexInFace) const
{
    return _vertexIndices[face * 3 + vertexInFace];
}
tSigned32
cFaceVertexMesh::vertexX(tSigned32 vertex) const
{
    return _vertexCoords[vertex * 3 + 0];
}
tSigned32
cFaceVertexMesh::vertexY(tSigned32 vertex) const
{
    return _vertexCoords[vertex * 3 + 1];
}
float
cFaceVertexMesh::vertexZ(tSigned32 vertex) const
{
    return static_cast<float>(_vertexCoords[vertex * 3 + 2]);
}
tSigned32
cFaceVertexMesh::faceAttribute(tSigned32, tSigned32) const
{
    return -1;
}
