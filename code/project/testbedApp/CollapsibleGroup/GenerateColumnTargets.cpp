#include "base/Header.h"
#include "project/testbedApp/CollapsibleGroup/GenerateColumnTargets.h"
#include "sampleShared/IntegerPoint2D.h"
#include "externalAPI/i_pathengine.h"
#include <vector>
#include <math.h>

static void
NormaliseToDoubles(const cIntegerPoint2D& v, double& x, double& y)
{
    // (convert to double before multiply)
    x = v.getX();
    y = v.getY();
    double lengthSquared = x * x + y * y;
    double length = sqrt(lengthSquared);
    x /= length;
    y /= length;
}

static void
GenerateColumnTargets(
        const iMesh* mesh, 
        iShape* agentShape, 
        const iCollisionContext* context,
        const cPosition& basePosition,
        const cIntegerPoint2D& incrementDirection,
        tSigned32 spacing,
        tSigned32 maximumWidth,
        std::vector<cPosition>& targets
        )
{
    double incrementX, incrementY;
    NormaliseToDoubles(incrementDirection, incrementX, incrementY);
    incrementX *= spacing;
    incrementY *= spacing;
    targets.clear();
    tSigned32 i;
    double x = basePosition.x;
    double y = basePosition.y;
    for(i = 1; i <= maximumWidth - 1 / 2; ++i)
    {
        x += incrementX;
        y += incrementY;
        cPosition p;
        p.x = static_cast<tSigned32>(x);
        p.y = static_cast<tSigned32>(y);
        if(mesh->testLineCollision_XY(agentShape, context, basePosition, p.x, p.y, p.cell))
        {
            return;
        }
        targets.push_back(p);
    }
}

void
GenerateColumnTargets(
        const iMesh* mesh, 
        iShape* agentShape, 
        const iCollisionContext* context,
        const cPosition& basePosition,
        tSigned32 forwardVectorX, tSigned32 forwardVectorY,
        tSigned32 spacing,
        tSigned32 maximumWidth,
        std::vector<cPosition>& leftTargets,
        std::vector<cPosition>& rightTargets
        )
{
    assertD(maximumWidth > 0);
    cIntegerPoint2D forwardVector(forwardVectorX, forwardVectorY);
    GenerateColumnTargets(mesh, agentShape, context, basePosition, forwardVector.left90(), spacing, maximumWidth, leftTargets);
    GenerateColumnTargets(mesh, agentShape, context, basePosition, forwardVector.right90(), spacing, maximumWidth, rightTargets);
}
