#ifndef CLUSTER_MOVEMENT_ORDER_INCLUDED
#define CLUSTER_MOVEMENT_ORDER_INCLUDED

#include "project/testbedApp/CollapsibleGroup/FormationMovementParameters.h"
#include <vector>

class iAgent;
class iPath;
class iMesh;
class iShape;
class iCollisionContext;
class cColumnTargetsManager;
class cPosition;
class iTestBed;

class cCollapsibleGroupAgent
{
    iShape* _agentShape;
    tSigned32 _agentIndex;

    enum eState
    {
        MOVING_TO_START,
        WAITING_FOR_SLOT,
        MOVING_TO_SLOT,
        AT_END,
        FAILED,
    } _state;

    tSigned32 _toRow, _toColumn;

  // when moving to start only
    iPath* _path;

  // when following base path only
    tSigned32 _fromRow, _fromColumn;
    double _currentLinkLength, _distanceRemaining;

    static double
    getSpacingRequired(
            cColumnTargetsManager& targetsManager,
            double groupStepDistance,
            tSigned32 toRow,
            tSigned32 toColumn
            );

    // returns true if moved
    // advanceDistance is reduced by amount actually advanced
    bool
    advanceStep(
            const iMesh* mesh,
            const iCollisionContext* context,
            cColumnTargetsManager& targetsManager,
            double groupStepDistance,
            double& advanceDistance,
            const cCollapsibleGroupAgent* leader,
            std::vector<cPosition>& positionsToUpdate
            );

public:

    cCollapsibleGroupAgent(
            const iMesh* mesh,
            const iCollisionContext* context,
            iShape* agentShape,
            tSigned32 agentIndex,
            const cPosition& agentStartPosition,
            cColumnTargetsManager& targetsManager,
            tSigned32 startRow, tSigned32 startColumn,
            bool& failed_NoPath
            );

    double
    distanceFromFrontRow(
            cColumnTargetsManager& targetsManager,
            double groupStepDistance
            ) const;

    bool
    isInFrontOf(const cCollapsibleGroupAgent& rhsAgent) const;

    // returns true if moved
    bool
    advance(
            const iMesh* mesh,
            const iCollisionContext* context,
            cColumnTargetsManager& targetsManager,
            double groupStepDistance,
            double advanceDistance,
            const cCollapsibleGroupAgent* leader, // don't advance past this leader agent
            std::vector<cPosition>& positionsToUpdate
            );

    void
    retire();

    void debugDraw(cColumnTargetsManager& targetsManager, iTestBed* testbed) const;
};

class cClusterMovementOrder
{
    const iMesh* _mesh;
    const iCollisionContext* _context;

    cColumnTargetsManager* _targetsManager;
    const cFormationMovementParameters _parameters;
    std::vector<cCollapsibleGroupAgent> _agents;

    // prevent copying and assignment
    cClusterMovementOrder(const cClusterMovementOrder&);
    const cClusterMovementOrder& operator=(const cClusterMovementOrder&);

public:

    cClusterMovementOrder(
            const iMesh* mesh,
            iShape* agentShape,
            const iCollisionContext* context,
            const cFormationMovementParameters& parameters,
            const std::vector<cPosition>& agentPositions,
            const cPosition& targetPosition,
            tSigned32& nextIndex,
            bool& failed_NoPath,            
            std::vector<tSigned32>& indexAssignments
            );
    ~cClusterMovementOrder();

    // returns true if reached end
    bool advance(std::vector<cPosition>& positionsToUpdate);

    void debugDraw(iTestBed* testbed) const;
};

#endif
