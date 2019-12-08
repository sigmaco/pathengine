#include "AddHavokConvexShape.h"
#include "SolidObjects.h"
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Convex/Triangle/hkpTriangleShape.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>
#include <Physics/Collide/Shape/Convex/ConvexTranslate/hkpConvexTranslateShape.h>
#include <Physics/Collide/Shape/Convex/ConvexTransform/hkpConvexTransformShape.h>
#include <Physics/Collide/Shape/Convex/Cylinder/hkpCylinderShape.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Common/Base/hkBase.h>
#include <vector>
//#include <windows.h>

template <class T> void
GetPoints(const hkpBoxShape& shape, T& addTo)
{
    hkVector4 halfExtents = shape.getHalfExtents();
    addTo.add(hkVector4(-halfExtents(0), -halfExtents(1), -halfExtents(2)));
    addTo.add(hkVector4(-halfExtents(0), halfExtents(1), -halfExtents(2)));
    addTo.add(hkVector4(halfExtents(0), halfExtents(1), -halfExtents(2)));
    addTo.add(hkVector4(halfExtents(0), -halfExtents(1), -halfExtents(2)));
    addTo.add(hkVector4(-halfExtents(0), -halfExtents(1), halfExtents(2)));
    addTo.add(hkVector4(-halfExtents(0), halfExtents(1), halfExtents(2)));
    addTo.add(hkVector4(halfExtents(0), halfExtents(1), halfExtents(2)));
    addTo.add(hkVector4(halfExtents(0), -halfExtents(1), halfExtents(2)));
}
template <class T> void
GetPoints(const hkpTriangleShape& shape, T& addTo)
{
    for(int i = 0; i != 3; ++i)
    {
        addTo.add(shape.getVertex(i));
    }
}
template <class T> void
GetPoints(const hkpConvexVerticesShape& shape, T& addTo)
{
    hkArray<hkVector4> vertices;
    shape.getOriginalVertices(vertices);
    for(int i = 0; i != vertices.getSize(); ++i)
    {
        addTo.add(vertices[i]);
    }
}
template <class T> void
GetPoints(const hkpCylinderShape& shape, T& addTo)
{
    // looks like it is not possible to query for the actual number of segments being used internally by Havok
    // (either default, or set by hkpCylinderShape::setNumberOfSideSegments())
    const tSigned32 numberOfVirtualSideSegments = 8;
    const hkVector4* axisVerts = shape.getVertices();
    hkVector4 axis = axisVerts[1];
    axis.sub4(axisVerts[0]);
    axis.normalize3();

  // get a couple of (arbitrary) perpendicular vectors
    hkVector4 perp1(0., 0., 0.);
    {
        int minorAxis = 0;
        if(axis(1) > axis(minorAxis))
        {
            minorAxis = 1;
        }
        if(axis(2) > axis(minorAxis))
        {
            minorAxis = 2;
        }
        perp1(minorAxis) = 1.;
    }
    {
        hkSimdReal dp = perp1.dot3(axis);
        hkSimdReal axisDP = axis.dot3(axis);
        hkSimdReal axisComponentRatio = dp / axisDP;
        hkVector4 axisComponent = axis;
        axisComponent.mul4(axisComponentRatio);
        perp1.sub4(axisComponent); // should make perp1 perpendicular to the axis
        perp1.normalize3();
    }

    hkVector4 perp2;
    perp2.setCross(axis, perp1);

  // premultiply these by desired radius
    hkReal radius = shape.getCylinderRadius();
    perp1.mul4(radius);
    perp2.mul4(radius);

    hkReal radianStep = static_cast<hkReal>(3.14159 * 2.0 / numberOfVirtualSideSegments);
    for(tSigned32 i = 0; i != numberOfVirtualSideSegments; ++i)
    {
        hkReal radians = radianStep * static_cast<hkReal>(i);
        hkVector4 perp1Component = perp1;
        hkVector4 perp2Component = perp2;
        perp1Component.mul4(hkMath::sin(radians));
        perp2Component.mul4(hkMath::cos(radians));
        hkVector4 perpsOffset = perp1Component;
        perpsOffset.add3clobberW(perp2Component);
        hkVector4 result = axisVerts[0];
        result.add3clobberW(perpsOffset);
        addTo.add(result);
        result = axisVerts[1];
        result.add3clobberW(perpsOffset);
        addTo.add(result);
    }
}

template <class T> void
GetPoints(const hkpSphereShape& shape, T& addTo)
{
    const tSigned32 stepsAround = 16;
    const tSigned32 stepsUp = 8;

    hkReal radius = shape.getRadius();

    {
        hkVector4 top(0, 0, radius);
        addTo.add(top);
        hkVector4 bottom(0, 0, -radius);
        addTo.add(bottom);
    }

    hkReal radianStepUp = static_cast<hkReal>(3.14159 / stepsUp);
    hkReal radianStepAround = static_cast<hkReal>(3.14159 * 2 / stepsAround);
    for(tSigned32 i = 1; i != stepsUp; ++i)
    {
        hkReal radiansUp = radianStepUp * static_cast<hkReal>(i);
        hkReal z = radius * hkMath::cos(radiansUp);
        hkReal xy = radius * hkMath::sin(radiansUp);
        for(tSigned32 j = 0; j != stepsAround; ++j)
        {
            hkReal radiansAround = radianStepAround * static_cast<hkReal>(j);
            hkReal x = xy * hkMath::sin(radiansAround);
            hkReal y = xy * hkMath::cos(radiansAround);
            hkVector4 p(x, y, z);
            addTo.add(p);
        }
    }
}

class cTransformedPoints
{
    hkTransform _transform;
    float _scale;
    std::vector<tSigned32> _points;
public:
    cTransformedPoints(const hkTransform& transform, float scale) :
      _transform(transform),
      _scale(scale)
    {
    }
    void
    add(const hkVector4& v4)
    {
        hkVector4 transformed;
        transformed.setTransformedPos(_transform, v4);
        transformed.mul4(_scale);
        _points.push_back(static_cast<tSigned32>(transformed(0)));
        _points.push_back(static_cast<tSigned32>(transformed(1)));
        _points.push_back(static_cast<tSigned32>(transformed(2)));
    }
    void
    addToSolidObjects(cSolidObjects& addTo)
    {
        addTo.add(_points);
    }
};

template <class T> void
AddHavokConvexShape_Template(const T& shape, const hkTransform& transform, float scale, cSolidObjects& addTo)
{
    cTransformedPoints transformedPoints(transform, scale);
    GetPoints(shape, transformedPoints);
    transformedPoints.addToSolidObjects(addTo);
}

void
AddHavokConvexShape(const hkpConvexShape& shape, const hkTransform& transform, float scale, cSolidObjects& addTo)
{
    hkpShapeType type = shape.getType();
    if(type == HK_SHAPE_BOX)
    {
        AddHavokConvexShape_Template(static_cast<const hkpBoxShape&>(shape), transform, scale, addTo);
    }
    else if(type == HK_SHAPE_TRIANGLE)
    {
        AddHavokConvexShape_Template(static_cast<const hkpTriangleShape&>(shape), transform, scale, addTo);
    }
    else if(type == HK_SHAPE_CONVEX_VERTICES)
    {
        AddHavokConvexShape_Template(static_cast<const hkpConvexVerticesShape&>(shape), transform, scale, addTo);
    }
    else if(type == HK_SHAPE_CYLINDER)
    {
        AddHavokConvexShape_Template(static_cast<const hkpCylinderShape&>(shape), transform, scale, addTo);
    }
    else if(type == HK_SHAPE_SPHERE)
    {
        AddHavokConvexShape_Template(static_cast<const hkpSphereShape&>(shape), transform, scale, addTo);
    }
    //else if(type == HK_SHAPE_CAPSULE)
    //{
    //    AddHavokConvexShape_Template(static_cast<const hkpCapsuleShape&>(shape), transform, scale, addTo);
    //}
    else if(type == HK_SHAPE_CONVEX_TRANSFORM)
    {
        const hkpConvexTransformShape& concreteShape = static_cast<const hkpConvexTransformShape&>(shape);
        const hkpConvexShape* childShape = concreteShape.getChildShape();
        hkTransform childTransform;
        childTransform.setMul(transform, concreteShape.getTransform());
        AddHavokConvexShape(*childShape, childTransform, scale, addTo);
    }
    else if(type == HK_SHAPE_CONVEX_TRANSLATE)
    {
        const hkpConvexTranslateShape& concreteShape = static_cast<const hkpConvexTranslateShape&>(shape);
        const hkpConvexShape* childShape = concreteShape.getChildShape();
        hkVector4 childTranslation = transform.getTranslation();
        childTranslation.add4(concreteShape.getTranslation());
        hkTransform childTransform;
        childTransform.setTranslation(childTranslation);
        childTransform.setRotation(transform.getRotation());
        AddHavokConvexShape(*childShape, childTransform, scale, addTo);
    }
    else
    {
        //OutputDebugString("shape type not supported\n");
    }
}
