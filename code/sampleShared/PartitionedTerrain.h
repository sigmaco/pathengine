
#pragma once

#include "externalAPI/i_pathengine.h"

class cPartitionedTerrain : public iFaceVertexMesh
{
    tSigned32 _originX, _originY;
    tSigned32 _squareSize, _squaresAcross;

public:

    cPartitionedTerrain(
            tSigned32 originX, tSigned32 originY,
            tSigned32 squareSize, tSigned32 squares
            ) :
     _originX(originX),
     _originY(originY),
     _squareSize(squareSize),
     _squaresAcross(squares)
    {
    }

// iFaceVertexMesh interface

    tSigned32 faces() const
    {
        return _squaresAcross * _squaresAcross * 2;
    }
    tSigned32 vertices() const
    {
        return (_squaresAcross + 1) * (_squaresAcross + 1);
    }
    tSigned32 vertexIndex(tSigned32 face, tSigned32 vertexInFace) const
    {
        tSigned32 baseVertex = face / 2 + face / 2 / _squaresAcross;
        if(face & 1)
        {
            switch(vertexInFace)
            {
            default:
//                invalid
            case 0:
                return baseVertex;
            case 1:
                return baseVertex + 1;
            case 2:
                return baseVertex + (_squaresAcross + 1) + 1;
            }
        }
        switch(vertexInFace)
        {
        default:
//            invalid
        case 0:
            return baseVertex;
        case 1:
            return baseVertex + (_squaresAcross + 1) + 1;
        case 2:
            return baseVertex + (_squaresAcross + 1);
        }
    }
    tSigned32 vertexX(tSigned32 vertex) const
    {
        return _originX + vertex / (_squaresAcross + 1) * _squareSize;
    }
    tSigned32 vertexY(tSigned32 vertex) const
    {
        return _originY + (vertex % (_squaresAcross + 1)) * _squareSize;
    }
    float vertexZ(tSigned32 vertex) const
    {
        return 0.0f;
    }
    tSigned32 faceAttribute(tSigned32 face, tSigned32 attributeIndex) const
    {
        if(attributeIndex == PE_FaceAttribute_SectionID)
        {
            return 0; // mark all faces as sectionID == 0 (first terrain layer)
        }
        return -1;
    }
};

