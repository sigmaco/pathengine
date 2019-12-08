#include "base/Header.h"
#include "project/testbedApp/CollapsibleGroup/GetGroupRootPosition.h"
#include "externalAPI/i_pathengine.h"

static double
DistanceSquared(const cPosition& p, double x, double y)
{
    return (x - p.x) * (x - p.x) + (y - p.y) * (y - p.y);
}

void
GetGroupRootPosition(
        const iMesh* mesh,
        const iCollisionContext* context,
        iShape* shape,
        const std::vector<cPosition>& groupPositions,
        cPosition& result
        )
{
    assertD(!groupPositions.empty());

  // get average X,Y
    double averageX, averageY;
    averageX = groupPositions[0].x;
    averageY = groupPositions[0].y;
    tSigned32 i;
    for(i = 1; i != groupPositions.size(); ++i)
    {
        averageX += groupPositions[i].x;
        averageY += groupPositions[i].y;
    }
    averageX /= groupPositions.size();
    averageY /= groupPositions.size();
    tSigned32 averageX32, averageY32;
    averageX32 = static_cast<tSigned32>(averageX);
    averageY32 = static_cast<tSigned32>(averageY);

  // find agent closest to centre
  // (this agent's ground position will then be used to root the group in the mesh)
    tSigned32 closestI = 0;
    double closestD = DistanceSquared(groupPositions[0], averageX, averageY);
    for(i = 1; i != groupPositions.size(); ++i)
    {
        double candidateD = DistanceSquared(groupPositions[i], averageX, averageY);
        if(candidateD < closestD)
        {
            closestI = i;
            closestD = candidateD;
        }
    }

    // using the position of the agent closest to centre, directly, guarantees that at least one agent should be able to reach this position
    result = groupPositions[closestI];
}
