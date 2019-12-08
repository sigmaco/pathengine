#include "base/Header.h"
#include "project/testbedApp/CollapsibleGroup/ClusterMovementOrder.h"
#include "project/testbedApp/CollapsibleGroup/GetGroupRootPosition.h"
#include "project/testbedApp/CollapsibleGroup/ColumnTargetsManager.h"
#include "project/testbedApp/CollapsibleGroup/FormationMovementParameters.h"
#include "project/testbedApp/CollapsibleGroup/AssignTargets.h"
#include "externalAPI/i_testbed.h"
#include "externalAPI/i_pathengine.h"
#include <math.h>
#include <algorithm>

static tSigned32
ColumnPriority(tSigned32 column)
{
    if(column >= 0)
    {
        return column * 2;
    }
    return (-1 - column) * 2 + 1;
}

static double
Distance(const cPosition& p1, const cPosition& p2)
{
    double dx = static_cast<double>(p1.x) - p2.x;
    double dy = static_cast<double>(p1.y) - p2.y;
    return sqrt(dx * dx + dy * dy);
}

static void
InterpolatePosition(
        const iMesh* mesh,
        const cPosition& p1, const cPosition& p2,
        double ratio,
        cPosition& interpolatedPosition
        )
{
    double dx = static_cast<double>(p2.x) - p1.x;
    double dy = static_cast<double>(p2.y) - p1.y;
    interpolatedPosition.x = p1.x + static_cast<tSigned32>(dx * ratio);
    interpolatedPosition.y = p1.y + static_cast<tSigned32>(dy * ratio);
    interpolatedPosition.cell = mesh->getCellForEndOfLine(p1, interpolatedPosition.x, interpolatedPosition.y);
    if(interpolatedPosition.cell == -1)
    {
        interpolatedPosition = p1;
    }
}

class cAgentInFrontPredicate
{
    const std::vector<cCollapsibleGroupAgent>* _agents;
public:
    cAgentInFrontPredicate(const std::vector<cCollapsibleGroupAgent>* agents)
    {
        _agents = agents;
    }
    bool
    operator()(tSigned32 lhs, tSigned32 rhs) const
    {
        return (*_agents)[lhs].isInFrontOf((*_agents)[rhs]);
    }
};

double
cCollapsibleGroupAgent::getSpacingRequired(
        cColumnTargetsManager& targetsManager,
        double groupStepDistance,
        tSigned32 toRow,
        tSigned32 toColumn
        )
{
    assertD(toRow <= targetsManager.frontRowIndex());
    if(toRow >= targetsManager.frontRowIndex())
    {
        return 0.;
    }
    cRowTargets& afterRow = targetsManager.refRow(toRow + 1);
    if(!afterRow.isValidColumn(toColumn))
    {
        return 0.;
    }
    const cCollapsibleGroupAgent* agentInFront = afterRow.assignedAgent(toColumn);
    if(!agentInFront)
    {
        return 0.;
    }
    double agentInFrontTravelled = agentInFront->_currentLinkLength - agentInFront->_distanceRemaining;
    return groupStepDistance - agentInFrontTravelled;
}

bool
cCollapsibleGroupAgent::advanceStep(
        const iMesh* mesh,
        const iCollisionContext* context,
        cColumnTargetsManager& targetsManager,
        double groupStepDistance,
        double& advanceDistance,
        const cCollapsibleGroupAgent* leader,
        std::vector<cPosition>& positionsToUpdate
        )
{
    if(_state == AT_END)
    {
        advanceDistance = 0.f;
        return false;
    }
    if(_state == MOVING_TO_START)
    {
        assertD(_path);
        {
            float precisionX, precisionY;
            _path->advanceAlong(_agentShape, static_cast<float>(advanceDistance), context, precisionX, precisionY);
        }
        positionsToUpdate[_agentIndex] = _path->position(0);
        if(_path->size() == 1 && !targetsManager.refRow(_toRow).isAssigned(_toColumn))
        {
            delete _path;
            _path = 0;
            targetsManager.refRow(_toRow).assign(_toColumn, this); // we depend on the address of this object remaining constant
            _state = WAITING_FOR_SLOT;
            _fromRow = _toRow;
            _fromColumn = _toColumn;
        }
        advanceDistance = 0.f;
        return true;
    }
    assertD(_path == 0);
    if(_state == MOVING_TO_SLOT)
    {
        cPosition toPosition = targetsManager.refRow(_toRow).position(_toColumn);
        if(_distanceRemaining <= advanceDistance)
        {
            _distanceRemaining = 0.f;
            advanceDistance = advanceDistance - _distanceRemaining;
            assertD(advanceDistance >= 0.f);
        }
        else
        {
            _distanceRemaining -= advanceDistance;
            advanceDistance = 0.f;
        }
        if(leader && leader->_fromRow == _fromRow && leader->_state == MOVING_TO_SLOT && _distanceRemaining < leader->_distanceRemaining)
        {
            _distanceRemaining = leader->_distanceRemaining;
            advanceDistance = 0.f;
        }
        double spacingRequired = getSpacingRequired(targetsManager, groupStepDistance, _toRow, _toColumn);
        if(_distanceRemaining < spacingRequired)
        {
            _distanceRemaining = spacingRequired;
            advanceDistance = 0.f;
        }

        if(_distanceRemaining == 0.f)
        {
            positionsToUpdate[_agentIndex] = toPosition;
            _state = WAITING_FOR_SLOT;
            _fromRow = _toRow;
            _fromColumn = _toColumn;
        }
        else
        {
            cPosition fromPosition = targetsManager.refRow(_fromRow).position(_fromColumn);
            cPosition interpolatedPosition;
            InterpolatePosition(mesh, toPosition, fromPosition, _distanceRemaining / _currentLinkLength, interpolatedPosition);
            positionsToUpdate[_agentIndex] = interpolatedPosition;
        }
        return true;
    }
    assertD(_state == WAITING_FOR_SLOT);

    if(leader && leader->_fromRow == _fromRow && leader->_state == WAITING_FOR_SLOT)
    {
        advanceDistance = 0.f;
        return false;
    }

    _toRow = _fromRow + 1;
    if(_toRow > targetsManager.frontRowIndex())
    {
        if(targetsManager.atEnd())
        {
            _state = AT_END;
            return false;
        }
        targetsManager.pushFront();
    }

    cRowTargets& fromRow = targetsManager.refRow(_fromRow);
    cRowTargets& toRow = targetsManager.refRow(_toRow);

    tSigned32 preferredSlots[3];
    if(_fromColumn < 0)
    {
        preferredSlots[0] = _fromColumn + 1;
        preferredSlots[1] = _fromColumn;
        preferredSlots[2] = _fromColumn - 1;
    }
    else
    if(_fromColumn > 0)
    {
        preferredSlots[0] = _fromColumn - 1;
        preferredSlots[1] = _fromColumn;
        preferredSlots[2] = _fromColumn + 1;
    }
    else
    {
        preferredSlots[0] = _fromColumn;
        preferredSlots[1] = _fromColumn + 1;
        preferredSlots[2] = _fromColumn - 1;
    }
    preferredSlots[0] = toRow.closestValidColumn(preferredSlots[0]);

    //if(_fromColumn && fromRow.centringAndWidthAreIdentical(toRow))
    //{
    //    if(fromRow.isAssigned(preferredSlots[0]) || toRow.isAssigned(preferredSlots[0]))
    //    {
    //      // this case added to try to get agents to 'lock in' to formation when no changes in corridor width
    //        // following stops agents replacing inside agent, but allows overtake
    //        preferredSlots[0] = _fromColumn;
    //        assertD(preferredSlots[1] == _fromColumn);
    //    }
    //}

    std::vector<tSigned32> overtakingSlots;
    std::vector<tSigned32> nonOvertakingSlots;
    tSigned32 i;
    for(i = 0; i != 3; ++i)
    {
        tSigned32 targetColumn = preferredSlots[i];
        if(!toRow.isValidColumn(targetColumn))
        {
            continue;
        }
        if(toRow.isAssigned(targetColumn))
        {
            continue;
        }
        double spacingRequired = getSpacingRequired(targetsManager, groupStepDistance, _toRow, targetColumn);
        if(spacingRequired > 0.)
        {
            nonOvertakingSlots.push_back(targetColumn);
        }
        else
        {
            overtakingSlots.push_back(targetColumn);
        }
    }

    std::vector<tSigned32> orderedSlots = overtakingSlots;
    for(i = 0; i != SizeL(nonOvertakingSlots); ++i)
    {
        orderedSlots.push_back(nonOvertakingSlots[i]);
    }

    if(!orderedSlots.empty())
    {
      // use best open slot
        tSigned32 targetColumn = orderedSlots[0];
        toRow.assign(targetColumn, this); // we depend on the address of this object remaining constant
        _toColumn = targetColumn;
        fromRow.unassign(_fromColumn, this); // we depend on the address of this object remaining constant
        _state = MOVING_TO_SLOT;
        {
            cPosition toPosition = toRow.position(_toColumn);
            cPosition fromPosition = fromRow.position(_fromColumn);
            _currentLinkLength = Distance(fromPosition, toPosition);
        }
        _distanceRemaining = _currentLinkLength;
        return true;
    }

    // didn't find a slot yet
    advanceDistance = 0.f; // can't advance for this update
    return false;
}

cCollapsibleGroupAgent::cCollapsibleGroupAgent(
        const iMesh* mesh,
        const iCollisionContext* context,
        iShape* agentShape,
        tSigned32 agentIndex,
        const cPosition& agentStartPosition,
        cColumnTargetsManager& targetsManager,
        tSigned32 startRow, tSigned32 startColumn,
        bool& failed_NoPath
        )
{
    _agentShape = agentShape;
    _agentIndex = agentIndex;
    _state = MOVING_TO_START;
    _toRow = startRow;
    _toColumn = startColumn;
    cPosition startPositionInFormation = targetsManager.refRow(startRow).position(startColumn);
    if(mesh->testPointCollision(agentShape, 0, agentStartPosition))
    {
        _path = 0;
    }
    else
    {
        _path = mesh->findShortestPath(agentShape, context, agentStartPosition, startPositionInFormation);
    }
    failed_NoPath = false;
    if(!_path)
    {
        _state = FAILED;
        failed_NoPath = true;
    }

}

double
cCollapsibleGroupAgent::distanceFromFrontRow(
        cColumnTargetsManager& targetsManager,
        double groupStepDistance
        ) const
{
    tSigned32 row;
    double distanceToRow = 0.f;
    if(_state == MOVING_TO_START)
    {
        row = _toRow;
        distanceToRow = _path->getLength();
    }
    else
    if(_state == MOVING_TO_SLOT)
    {
        row = _toRow;
        distanceToRow = _distanceRemaining;
    }
    else
    {
        row = _fromRow;
        distanceToRow = 0.f;
    }
    assertD(targetsManager.frontRowIndex() >= row);
    return distanceToRow + (targetsManager.frontRowIndex() - row) * groupStepDistance;
}

bool
cCollapsibleGroupAgent::isInFrontOf(const cCollapsibleGroupAgent& rhsAgent) const
{
    const cCollapsibleGroupAgent& lhsAgent = *this;
    bool lhsReachedStart = (lhsAgent._state != MOVING_TO_START);
    bool rhsReachedStart = (rhsAgent._state != MOVING_TO_START);
    if(lhsReachedStart != rhsReachedStart)
    {
        return lhsReachedStart;
    }
    if(!lhsReachedStart)
    {
        return _path->getLength() < rhsAgent._path->getLength();
    }
    if(_toRow != rhsAgent._toRow)
    {
        return _toRow > rhsAgent._toRow;
    }
    //return _distanceRemaining < rhsAgent._distanceRemaining;
    return ColumnPriority(_toColumn) < ColumnPriority(rhsAgent._toColumn);
}

bool
cCollapsibleGroupAgent::advance(
        const iMesh* mesh,
        const iCollisionContext* context,
        cColumnTargetsManager& targetsManager,
        double groupStepDistance,
        double advanceDistance,
        const cCollapsibleGroupAgent* leader,
        std::vector<cPosition>& positionsToUpdate
        )
{
    if(leader == this)
    {
        leader = 0;
    }
    bool moved = false;
    assertD(advanceDistance > 0.f);
    do
    {
        if(advanceStep(mesh, context, targetsManager, groupStepDistance, advanceDistance, leader, positionsToUpdate))
        {
            moved = true;
        }
    }
    while(advanceDistance > 0.f);
    return moved;
}

void
cCollapsibleGroupAgent::retire()
{
    delete _path;
}

void
cCollapsibleGroupAgent::debugDraw(cColumnTargetsManager& targetsManager, iTestBed* testbed) const
{
    if(_state == MOVING_TO_START)
    {
        testbed->drawPath(_path);
    }
    else
    if(_state == MOVING_TO_SLOT)
    {
        cRowTargets& fromRow = targetsManager.refRow(_fromRow);
        cRowTargets& toRow = targetsManager.refRow(_toRow);
        testbed->drawLine(fromRow.position(_fromColumn), toRow.position(_toColumn));
    }
}

cClusterMovementOrder::cClusterMovementOrder(
        const iMesh* mesh,
        iShape* agentShape,
        const iCollisionContext* context,
        const cFormationMovementParameters& parameters,
        const std::vector<cPosition>& agentPositions,
        const cPosition& targetPosition,
        tSigned32& nextIndex,
        bool& failed_NoPath,            
        std::vector<tSigned32>& indexAssignments
        ) :
 _parameters(parameters)
{
    assertD(!agentPositions.empty());
    assertD(indexAssignments.empty());

    _targetsManager = 0;
    {
        cPosition startP;
        GetGroupRootPosition(mesh, context, agentShape, agentPositions, startP);
        if(startP.cell == -1) // should not happen unless agents are inside obstructed space for the context
        {
            failed_NoPath = true;
            return;
        }
        assertD(!mesh->testPointCollision(agentShape, 0, startP));
        iPath* path = mesh->findShortestPath(agentShape, context, startP, targetPosition);
        if(path == 0)
        {
            failed_NoPath = true;
            return;
        }
        _targetsManager = new cColumnTargetsManager(
                mesh,
                agentShape,
                context,
                path,
                parameters._agentSpacing, parameters._maximumWidth,
                parameters._groupStepDistance
                );
    }

    failed_NoPath = false;

    _mesh = mesh;
    _context = context;

    std::vector<tSigned32> startRows;
    std::vector<tSigned32> startColumns;
    std::vector<cPosition> startPositions;
    do
    {
        tSigned32 rowIndex = _targetsManager->pushFront();
        cRowTargets& row = _targetsManager->refRow(rowIndex);
        tSigned32 column = row.firstColumn();
        do
        {
            startRows.push_back(rowIndex);
            startColumns.push_back(column);
            startPositions.push_back(row.position(column));
            ++column;
        }
        while(column <= row.lastColumn() && SizeL(startPositions) < parameters._numberOfStartPositions);
    }
    while(!_targetsManager->atEnd() && SizeL(startPositions) < parameters._numberOfStartPositions);


    std::vector<tSigned32> assignments;
    AssignTargets(agentPositions, startPositions, assignments);

    indexAssignments.resize(agentPositions.size());
    tSigned32 i;
    for(i = 0; i != SizeL(agentPositions); ++i)
    {
        tSigned32 j = assignments[i];
        bool noPathForAgent;
        cCollapsibleGroupAgent cga(
                mesh, context,
                agentShape, nextIndex,
                agentPositions[i],
                *_targetsManager, startRows[j], startColumns[j],
                noPathForAgent
                );
        if(noPathForAgent)
        {
            indexAssignments[i] = -1;
        }
        else
        {
            indexAssignments[i] = nextIndex++;
            _agents.push_back(cga);
        }
    }


    //assertD(indexAssignments.empty());
    //_mesh = mesh;
    //_context = context;


    //indexAssignments.resize(SizeL(agentPositions));

    //_speedGroups.push_back(new cSpeedMatchedGroupMovementOrder(
    //        mesh, agentShape,
    //        context,
    //        _targetsManager,
    //        startRows, startColumns, startPositions,
    //        parameters,
    //        parameters._agentSpeed, //..... this is in parameters arg!
    //        agentPositions,
    //        nextIndex,
    //        indexAssignments
    //        ));

    ////tSigned32 i;
    ////i = 0;
    ////while(i != SizeL(agentPositions))
    ////{
    ////    std::vector<tSigned32> speedGroupAgents;
    ////    std::vector<cPosition> speedGroupPositions;
    ////    const double groupSpeed = agentSpeeds[speedIndex[i]];
    ////    do
    ////    {
    ////        tSigned32 agentI = speedIndex[i];
    ////        speedGroupAgents.push_back(agentI);
    ////        speedGroupPositions.push_back(agentPositions[agentI]);
    ////        ++i;
    ////    }
    ////    while(i < SizeL(speedIndex) && groupSpeed == agentSpeeds[speedIndex[i]]);

    ////    std::vector<tSigned32> speedGroupIndexAssignments;
    ////    _speedGroups.push_back(new cSpeedMatchedGroupMovementOrder(
    ////            mesh, agentShape,
    ////            context,
    ////            _targetsManager,
    ////            startRows, startColumns, startPositions,
    ////            parameters,
    ////            groupSpeed,
    ////            speedGroupPositions,
    ////            nextIndex,
    ////            speedGroupIndexAssignments
    ////            ));
    ////    for(tSigned32 j = 0; j != SizeL(speedGroupAgents); ++j)
    ////    {
    ////        indexAssignments[speedGroupAgents[j]] = speedGroupIndexAssignments[j];
    ////    }
    ////}
    //failed_NoPath = false;
}
cClusterMovementOrder::~cClusterMovementOrder()
{
    for(tSigned32 i = 0; i != SizeL(_agents); ++i)
    {
        _agents[i].retire();
    }
    delete _targetsManager;
}

bool
cClusterMovementOrder::advance(std::vector<cPosition>& positionsToUpdate)
{
    assertD(!_agents.empty());
    bool movedAtLeastOne = false;
    std::vector<tSigned32> agentOrder(_agents.size());
    tSigned32 i;
    for(i = 0; i != SizeL(_agents); ++i)
    {
        agentOrder[i] = i;
    }
    std::sort(agentOrder.begin(), agentOrder.end(), cAgentInFrontPredicate(&_agents));
    double leaderSpeed = _parameters._agentSpeed;
    double followersSpeed = leaderSpeed * 2.f;

    for(i = 0; i != SizeL(_agents); ++i)
    {
        double speed = i ? followersSpeed : leaderSpeed;
        if(speed == 0.f)
        {
            continue;
        }
        if(_agents[agentOrder[i]].advance(_mesh, _context, *_targetsManager, _parameters._groupStepDistance, speed, &_agents[agentOrder[0]], positionsToUpdate))
        {
            movedAtLeastOne = true;
        }
    }
    return !movedAtLeastOne;
}


void
cClusterMovementOrder::debugDraw(iTestBed* testbed) const
{
    for(tSigned32 i = 0; i != SizeL(_agents); ++i)
    {
        _agents[i].debugDraw(*_targetsManager, testbed);
    }
}
