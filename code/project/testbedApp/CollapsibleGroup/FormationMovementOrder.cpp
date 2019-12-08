#include "base/Header.h"
#include "project/testbedApp/CollapsibleGroup/ColumnTargetsManager.h"
#include "project/testbedApp/CollapsibleGroup/FormationMovementOrder.h"
#include "project/testbedApp/CollapsibleGroup/ClusterMovementOrder.h"
#include "project/testbedApp/CollapsibleGroup/AssignToDistanceBasedClusters.h"
#include "project/testbedApp/CollapsibleGroup/FormationMovementParameters.h"

cFormationMovementOrder::cFormationMovementOrder(
        const iMesh* mesh,
        iShape* agentShape,
        const iCollisionContext* context,
        const cFormationMovementParameters& parameters,
        const std::vector<cPosition>& agentStartPositions,
        const cPosition& targetPosition,
        std::vector<tSigned32>& agentIndexAssignments
        )
{
    assertD(agentIndexAssignments.empty());
    agentIndexAssignments.resize(SizeL(agentStartPositions), -1);
    std::vector<tSigned32> toBeAssigned(SizeL(agentStartPositions));
    tSigned32 i;
    for(i = 0; i != SizeL(toBeAssigned); ++i)
    {
        toBeAssigned[i] = i;
    }

    // incremented as index values are used in each sub order
    tSigned32 nextIndex = 0;

  // generate distance based clusters and build per-cluster movement orders, until no more agents to assign
  // in cases where some agents fail to reach the starting position for their movement orders,
  // these can then form up into new clusters
  // (because clusters are purely distance based, it is potentially possible for these to span multiple disconnected regions in pathfinding space)
    while(!toBeAssigned.empty())
    {
        std::vector<tSigned32> assignedAgents;
        std::vector<cPosition> assignedPositions;
        {
            std::vector<cPosition> positions(toBeAssigned.size());
            for(i = 0; i != SizeL(positions); ++i)
            {
                positions[i] = agentStartPositions[toBeAssigned[i]];
            }
            std::vector<tSigned32> assigned;
            std::vector<tSigned32> notAssigned;
            AssignOneDistanceBasedCluster(positions, parameters._maximumClusterRange, assigned, notAssigned);
            for(i = 0; i != SizeL(assigned); ++i)
            {
                tSigned32 agentI = toBeAssigned[assigned[i]];
                assignedAgents.push_back(agentI);
                assignedPositions.push_back(agentStartPositions[agentI]);
            }
            std::vector<tSigned32> notAssignedAgents;
            for(i = 0; i != SizeL(notAssigned); ++i)
            {
                notAssignedAgents.push_back(toBeAssigned[notAssigned[i]]);
            }
            toBeAssigned = notAssignedAgents;
        }
        assertD(!assignedAgents.empty());

        bool failed_NoPath;
        std::vector<tSigned32> failedAgents;
        std::vector<tSigned32> indexAssignments;
        cClusterMovementOrder* order = new cClusterMovementOrder(
                mesh, agentShape, context,
                parameters,
                assignedPositions,
                targetPosition,
                nextIndex,
                failed_NoPath,
                indexAssignments
                );

        if(failed_NoPath)
        {
            delete order;
            continue;
        }
        tSigned32 numberSucceeded = 0;
        for(i = 0; i != SizeL(indexAssignments); ++i)
        {
            if(indexAssignments[i] != -1)
            {
                agentIndexAssignments[assignedAgents[i]] = indexAssignments[i];
                ++numberSucceeded;
            }
        }
        if(numberSucceeded == 0)
        {
            delete order;
            continue;
        }
        // not all agents failed
        // in this case, any failed agents are added back in to be potentially assigned to other clusters
        for(i = 0; i != SizeL(indexAssignments); ++i)
        {
            if(indexAssignments[i] == -1)
            {
                toBeAssigned.push_back(assignedAgents[i]);
            }
        }
        _clusterOrders.push_back(order);
    }

    _currentPositions.resize(nextIndex);
    for(i = 0; i != SizeL(agentStartPositions); ++i)
    {
        if(agentIndexAssignments[i] != -1)
        {
            _currentPositions[i] = agentStartPositions[agentIndexAssignments[i]];
        }
    }
}

cFormationMovementOrder::~cFormationMovementOrder()
{
    tSigned32 i;
    for(i = 0; i != SizeL(_clusterOrders); ++i)
    {
        delete _clusterOrders[i];
    }
}

bool
cFormationMovementOrder::advance()
{
    std::vector<cClusterMovementOrder*> activeOrders;
    tSigned32 i;
    for(i = 0; i != SizeL(_clusterOrders); ++i)
    {
        bool clusterReachedEnd = _clusterOrders[i]->advance(_currentPositions);
        if(clusterReachedEnd)
        {
            delete _clusterOrders[i];
        }
        else
        {
            activeOrders.push_back(_clusterOrders[i]);
        }
    }
    _clusterOrders.swap(activeOrders);
    return _clusterOrders.empty();
}

cPosition
cFormationMovementOrder::getCurrentPosition(tSigned32 agentIndex) const
{
    assertD(agentIndex >= 0 && agentIndex < SizeL(_currentPositions));
    return _currentPositions[agentIndex];
}

void
cFormationMovementOrder::debugDraw(iTestBed* testbed) const
{
    tSigned32 i;
    for(i = 0; i != SizeL(_clusterOrders); ++i)
    {
        _clusterOrders[i]->debugDraw(testbed);
    }
}

