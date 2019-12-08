#ifndef FORMATION_MOVEMENT_ORDER_INCLUDED
#define FORMATION_MOVEMENT_ORDER_INCLUDED

#include <vector>

class iAgent;
class iMesh;
class iShape;
class iCollisionContext;
class cPosition;
class iTestBed;
class cClusterMovementOrder;
class cFormationMovementParameters;

class cFormationMovementOrder
{
    std::vector<cClusterMovementOrder*> _clusterOrders;
    std::vector<cPosition> _currentPositions;

    // prevent copying and assignment
    cFormationMovementOrder(const cFormationMovementOrder&);
    const cFormationMovementOrder& operator=(const cFormationMovementOrder&);

public:

    cFormationMovementOrder(
            const iMesh* mesh,
            iShape* agentShape,
            const iCollisionContext* context,
            const cFormationMovementParameters& parameters,
            const std::vector<cPosition>& agentStartPositions,
            const cPosition& targetPosition,
            std::vector<tSigned32>& agentIndexAssignments // per agent in order, for querying position, -1 indicates agent cannot reach target
            );
    ~cFormationMovementOrder();

    // returns true if reached end
    bool advance();

    cPosition getCurrentPosition(tSigned32 agentIndex) const;

    void debugDraw(iTestBed* testbed) const;
};

#endif
