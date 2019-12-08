#include "PhysXTerrainWrapper.h"
#include "CoordinateMapping.h"
#include <NxHeightFieldShape.h>
#include <NxHeightFieldShapeDesc.h>
#include <NxHeightField.h>
#include <NxHeightFieldDesc.h>
#include <NxMat34.h>
#include <NxHeightFieldSample.h> 

cPhysXTerrainWrapper::cPhysXTerrainWrapper(const NxHeightFieldShape& heightFieldShape, const cCoordinateMapping& coordinateMapping, float scale)
{
    NxMat34 transform = heightFieldShape.getGlobalPose();
    transform.M *= scale;
    transform.t *= scale;

    NxHeightFieldShapeDesc heightFieldShapeDesc;
    heightFieldShape.saveToDesc(heightFieldShapeDesc);

    const NxHeightField& heightField = heightFieldShape.getHeightField();

    NxHeightFieldDesc heightFieldDesc;
    heightField.saveToDesc(heightFieldDesc);

    _stride = heightFieldDesc.nbColumns;
    _strides = heightFieldDesc.nbRows;
    tSigned32 points = _stride * _strides;
    std::vector<char> samplesBuffer(points * heightFieldDesc.sampleStride);
    {
        NxU32 written = heightField.saveCells(&samplesBuffer[0], points * heightFieldDesc.sampleStride);
        assert(written == points * heightFieldDesc.sampleStride);
        heightFieldDesc.samples = (void*)(&samplesBuffer[0]);
    }

    _pointCoords.reserve(points * 3);
    _tessellationFlags.reserve(points);
    const char* bytePtr = (const char*)(heightFieldDesc.samples);
    assert(heightFieldDesc.format == NX_HF_S16_TM);
    tSigned32 faceIndex = 0;
    for(tSigned32 row = 0; row != _strides; ++row)
    {
        NxVec3 localPoint;
        localPoint.x = static_cast<NxReal>(row) * heightFieldShapeDesc.rowScale;
        for(tSigned32 column = 0; column != _stride; ++column)
        {
            const NxHeightFieldSample* samplePtr = (NxHeightFieldSample*)bytePtr;
            const NxHeightFieldSample& sample = *samplePtr;
            localPoint.y = static_cast<NxReal>(sample.height) * heightFieldShapeDesc.heightScale;
            localPoint.z = static_cast<NxReal>(column) * heightFieldShapeDesc.rowScale;
            NxVec3 worldPoint;
            transform.multiply(localPoint, worldPoint);
            _pointCoords.push_back(static_cast<tSigned32>(worldPoint.x));
            _pointCoords.push_back(static_cast<tSigned32>(worldPoint.y));
            _pointCoords.push_back(static_cast<tSigned32>(worldPoint.z));
            _tessellationFlags.push_back(sample.tessFlag);
            if(row + 1 != _strides && column + 1 != _stride)
            {
                if(sample.materialIndex0 != heightFieldShapeDesc.holeMaterial)
                {
                    _holesRemap.push_back(faceIndex);
                }
                if(sample.materialIndex1 != heightFieldShapeDesc.holeMaterial)
                {
                    _holesRemap.push_back(faceIndex + 1);
                }
            }
            faceIndex += 2;
            bytePtr += heightFieldDesc.sampleStride;
        }
    }
    
    for(tSigned32 i = 0; i != points; ++i)
    {
        coordinateMapping.applyTo(&_pointCoords[i * 3]);
    }
}


// iFaceVertexMesh interface

tSigned32
cPhysXTerrainWrapper::faces() const
{
    return static_cast<tSigned32>(_holesRemap.size());
}
tSigned32
cPhysXTerrainWrapper::vertices() const
{
    assert(_stride * _strides * 3 == static_cast<tSigned32>(_pointCoords.size()));
    return _stride * _strides;
}
tSigned32
cPhysXTerrainWrapper::vertexIndex(tSigned32 face, tSigned32 vertexInFace) const
{
    assert(face >= 0 && face < static_cast<tSigned32>(_holesRemap.size()));
    tSigned32 remappedFace = _holesRemap[face];
    tSigned32 square = remappedFace / 2;
    tSigned32 faceInSquare = (remappedFace & 1);
    tSigned32 vertexInSquare;
    if(_tessellationFlags[square])
    {
        const tSigned32 vertLookup[] = {0, 3, 1, 1, 3, 2};
        vertexInSquare = vertLookup[faceInSquare * 3 + vertexInFace];
    }
    else
    {
        const tSigned32 vertLookup[] = {0, 2, 1, 0, 3, 2};
        vertexInSquare = vertLookup[faceInSquare * 3 + vertexInFace];
    }
    tSigned32 result = square;
    switch(vertexInSquare)
    {
    default:
    case 0:
        break;
    case 1:
        ++result;
        break;
    case 2:
        result += 1 + _stride;
        break;
    case 3:
        result += _stride;
        break;
    }
    return result;
}
tSigned32
cPhysXTerrainWrapper::vertexX(tSigned32 vertex) const
{
    return _pointCoords[vertex * 3];
}
tSigned32
cPhysXTerrainWrapper::vertexY(tSigned32 vertex) const
{
    return _pointCoords[vertex * 3 + 1];
}
float
cPhysXTerrainWrapper::vertexZ(tSigned32 vertex) const
{
    return static_cast<float>(_pointCoords[vertex * 3 + 2]);
}
tSigned32
cPhysXTerrainWrapper::faceAttribute(tSigned32 face, tSigned32 attributeIndex) const
{
    if(attributeIndex == PE_FaceAttribute_SectionID)
    {
        return 0; // mark all faces as sectionID == 0 (first terrain layer)
    }
    return -1;
}
