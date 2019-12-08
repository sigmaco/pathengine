#include "AddPhysXConvexShape.h"
#include "SolidObjects.h"
#include "i_pathengine.h"
#include <NxConvexShape.h>
#include <NxConvexMeshDesc.h>
#include <NxBoxShape.h>
#include <NxBoxShapeDesc.h>
#include <NxSphereShape.h>
#include <vector>
//#include <windows.h>

static void
GetPoints(const NxBoxShape& shape, float scale, std::vector<NxVec3>& addTo)
{
    NxMat34 transform = shape.getGlobalPose();
    transform.M *= scale;
    transform.t *= scale;
    NxBoxShapeDesc shapeDesc;
    shape.saveToDesc(shapeDesc);
    NxVec3 dimensions = shapeDesc.dimensions;
    for(tSigned32 i = 0; i != 8; ++i)
    {
        NxVec3 localPoint;
        localPoint.x = i < 4 ? -dimensions.x : dimensions.x;
        localPoint.y = (i & 3) < 2 ? -dimensions.y: dimensions.y;
        localPoint.z = (i & 1) < 1 ? -dimensions.z: dimensions.z;
        NxVec3 transformed;
        transform.multiply(localPoint, transformed);
        addTo.push_back(transformed);
    }
}

static void
GetPoints(const NxConvexShape& shape, float scale, std::vector<NxVec3>& addTo)
{
    NxMat34 transform = shape.getGlobalPose();
    transform.M *= scale;
    transform.t *= scale;
    NxConvexMeshDesc shapeDesc;
    shape.getConvexMesh().saveToDesc(shapeDesc);
    const char* bytePtr = (const char*)(shapeDesc.points);
    tSigned32 numberOfPoints = static_cast<tSigned32>(shapeDesc.numVertices);
    for(tSigned32 i = 0; i != numberOfPoints; ++i)
    {
        const NxVec3* data = (const NxVec3*)(bytePtr + i * shapeDesc.pointStrideBytes);
        const NxVec3& localPoint = *data;
        NxVec3 transformed;
        transform.multiply(localPoint, transformed);
        addTo.push_back(transformed);
    }
}

static void
GetPoints(const NxSphereShape& shape, float scale, std::vector<NxVec3>& addTo)
{
    NxMat34 transform = shape.getGlobalPose();
    transform.M *= scale;
    transform.t *= scale;
    NxReal radius = shape.getRadius();

    const tSigned32 stepsAround = 16;
    const tSigned32 stepsUp = 8;

    NxVec3 transformed;
    {
        const NxVec3 top(0, 0, radius);
        transform.multiply(top, transformed);
        addTo.push_back(transformed);
    }
    {
        const NxVec3 bottom(0, 0, -radius);
        transform.multiply(bottom, transformed);
        addTo.push_back(transformed);
    }

    NxReal radianStepUp = static_cast<NxReal>(3.14159 / stepsUp);
    NxReal radianStepAround = static_cast<NxReal>(3.14159 * 2 / stepsAround);
    for(tSigned32 i = 1; i != stepsUp; ++i)
    {
        NxReal radiansUp = radianStepUp * static_cast<NxReal>(i);
        NxReal z = radius * NxMath::cos(radiansUp);
        NxReal xy = radius * NxMath::sin(radiansUp);
        for(tSigned32 j = 0; j != stepsAround; ++j)
        {
            NxReal radiansAround = radianStepAround * static_cast<NxReal>(j);
            NxReal x = xy * NxMath::sin(radiansAround);
            NxReal y = xy * NxMath::cos(radiansAround);
            const NxVec3 local(x, y, z);
            transform.multiply(local, transformed);
            addTo.push_back(transformed);
        }
    }
}


bool
AddPhysXConvexShape(const NxShape& shape, float scale, cSolidObjects& addTo)
{
    std::vector<NxVec3> points;
    NxShapeType type = shape.getType();
    if(type == NX_SHAPE_BOX)
    {
        GetPoints(*shape.isBox(), scale, points);
    }
    else if(type == NX_SHAPE_CONVEX)
    {
        GetPoints(*shape.isConvexMesh(), scale, points);
    }
    else if(type == NX_SHAPE_SPHERE)
    {
        GetPoints(*shape.isSphere(), scale, points);
    }
    else
    {
        return false;
    }

    std::vector<tSigned32> integerCoords(points.size() * 3);
    tSigned32 i;
    for(i = 0; i != points.size(); ++i)
    {
        integerCoords[i * 3 + 0] = static_cast<tSigned32>(points[i].x);
        integerCoords[i * 3 + 1] = static_cast<tSigned32>(points[i].y);
        integerCoords[i * 3 + 2] = static_cast<tSigned32>(points[i].z);
    }
    addTo.add(integerCoords);

    return true;
}
