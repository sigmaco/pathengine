//**********************************************************************
//
// Copyright (c) 2012
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include "base/Header.h"
#include "project/testbedApp/SeparatingGroups/AgentAI.h"
#include "externalAPI/i_pathengine.h"

class cMovementOrder
{
    std::vector<cPosition> _waypoints;
    bool _repeat;
    tSigned32 _nextWaypoint;
    tSigned32 _numberInProgress;
public:
    cMovementOrder(
            const std::vector<cPosition>& waypoints,
            bool repeat
            ) :
     _waypoints(waypoints)
    {
        assertD(!_waypoints.empty());
        _repeat = repeat;
        _numberInProgress = 0;
        _nextWaypoint = 0;
    }
    bool
    canProceed() const
    {
        return _numberInProgress == 0;
    }
    bool
    isComplete() const
    {
        assertD(canProceed());
        return _nextWaypoint == SizeL(_waypoints);
    }
    cPosition
    getNextTarget(tSigned32 numberOfAgents)
    {
        assertD(canProceed());
        assertD(!isComplete());
        cPosition result = _waypoints[_nextWaypoint];
        ++_nextWaypoint;
        if(_repeat && _nextWaypoint == SizeL(_waypoints))
        {
            _nextWaypoint = 0;
        }
        _numberInProgress = numberOfAgents;
        return result;
    }
    void
    notifyAgentCompletedOrAbandonedWaypoint()
    {
        assertD(_numberInProgress);
        _numberInProgress--;
    }
};

void
cAgentAI::pathsToSingleTarget(const std::vector<tSigned32>& selectedAgents, const cPosition& targetPosition)
{
    // abort current paths if any
    for(tSigned32 j = 0; j != SizeL(selectedAgents); ++j)
    {
        tSigned32 i = selectedAgents[j];
        delete _agentPaths[i];
        _agentPaths[i] = 0;
    }

    // remove all from at rest context
    for(tSigned32 j = 0; j != SizeL(selectedAgents); ++j)
    {
        tSigned32 i = selectedAgents[j];
        if(_atRestContext->includes(_agents[i]))
        {
            _atRestContext->removeAgent(_agents[i]);
            _avoidContext->addAgent(_avoidAgents[i]);
        }
    }

    // and then do pathfinding
    for(tSigned32 j = 0; j != SizeL(selectedAgents); ++j)
    {
        tSigned32 i = selectedAgents[j];
        _agentPaths[i] = _agents[i]->findShortestPathTo(_atRestContext, targetPosition);
        if(!_agentPaths[i])
        {
            _atRestContext->addAgent(_agents[i]);
            _avoidContext->removeAgent(_avoidAgents[i]);
        }
    }
}

cAgentAI::cAgentAI(iMesh* mesh, iShape* agentShape, iShape* avoidShape)
{
    _mesh = mesh;
    _agentShape = agentShape;
    _avoidShape = avoidShape;
    _atRestContext = _mesh->newContext();
    _avoidContext = _mesh->newContext();

    cPosition placementCentre;
    do
    {
        placementCentre = mesh->generateRandomPosition();
    }
    while(_mesh->testPointCollision(_agentShape, 0, placementCentre));
    for(tSigned32 i = 0; i != 20; ++i)
    {
        cPosition p;
        do
        {
            p = _mesh->generateRandomPositionLocally(placementCentre, 1000);
        }
        while(_mesh->testPointCollision(_agentShape, _atRestContext, p));
        _agents.push_back(_mesh->placeAgent(_agentShape, p));
        _avoidAgents.push_back(_mesh->placeAgent(_avoidShape, p));
        _agents.back()->setUserData(i);
        _avoidAgents.back()->setUserData(i);
        _agentPaths.push_back(0);
        _atRestContext->addAgent(_agents.back());
    }
    _agentMovementOrders.resize(_agents.size(), -1);
}

cAgentAI::~cAgentAI()
{
    for(tSigned32 i = 0; i != SizeL(_movementOrders); ++i)
    {
        delete _movementOrders[i];
    }
}

void
cAgentAI::stopOrder(const std::vector<tSigned32>& selectedAgents)
{
    for(tSigned32 j = 0; j != SizeL(selectedAgents); ++j)
    {
        tSigned32 i = selectedAgents[j];
        delete _agentPaths[i];
        _agentPaths[i] = 0;
        if(_agentMovementOrders[i] != -1)
        {
            _movementOrders[_agentMovementOrders[i]]->notifyAgentCompletedOrAbandonedWaypoint();
            _agentMovementOrders[i] = -1;
        }
        if(!_atRestContext->includes(_agents[i]))
        {
            _atRestContext->addAgent(_agents[i]);
            _avoidContext->removeAgent(_avoidAgents[i]);
        }
    }
}
void
cAgentAI::moveOrder(const std::vector<tSigned32>& selectedAgents, const cPosition& targetPosition)
{
    pathsToSingleTarget(selectedAgents, targetPosition);

    // notify existing movement orders about abandon, if any
    for(tSigned32 j = 0; j != SizeL(selectedAgents); ++j)
    {
        tSigned32 i = selectedAgents[j];
        if(_agentMovementOrders[i] != -1)
        {
            _movementOrders[_agentMovementOrders[i]]->notifyAgentCompletedOrAbandonedWaypoint();
            _agentMovementOrders[i] = -1;
        }
    }
}

void
cAgentAI::moveOrder(const std::vector<tSigned32>& selectedAgents, const std::vector<cPosition>& waypoints, bool repeat)
{
    stopOrder(selectedAgents);

  // get a free slot for the new movement order

    tSigned32 slot;
    for(slot = 0; slot != SizeL(_movementOrders); ++slot)
    {
        if(!_movementOrders[slot])
        {
            break;
        }
    }
    if(slot == SizeL(_movementOrders))
    {
        _movementOrders.push_back(0);
    }

  // create the movement order

    _movementOrders[slot] = new cMovementOrder(waypoints, repeat);

  // assign to the selected agents

    for(tSigned32 j = 0; j != SizeL(selectedAgents); ++j)
    {
        tSigned32 i = selectedAgents[j];
        _agentMovementOrders[i] = slot;
    }
}

void
cAgentAI::update()
{
    const float agentSpeed = 16.f;
    const float agentSpeed_Queuing = 10.f;
    const tSigned32 positionResolveDistance = 1500;
    tSigned32 i;

    // update movement orders
    for(tSigned32 orderI = 0; orderI != SizeL(_movementOrders); ++orderI)
    {
        cMovementOrder* order = _movementOrders[orderI];
        if(order == 0 || !order->canProceed())
        {
            continue;
        }
        if(order->isComplete())
        {
            delete order;
            _movementOrders[orderI] = 0;
            for(i = 0; i != SizeL(_agents); ++i)
            {
                if(_agentMovementOrders[i] == orderI)
                {
                    assertD(_agentPaths[i] == 0);
                    _agentMovementOrders[i] = -1;
                }
            }
            continue;
        }
        std::vector<tSigned32> assignedAgents;
        for(i = 0; i != SizeL(_agents); ++i)
        {
            if(_agentMovementOrders[i] == orderI)
            {
                assignedAgents.push_back(i);
            }
        }
        if(assignedAgents.empty())
        {
            delete order;
            _movementOrders[orderI] = 0;
            continue;
        }
        cPosition target = order->getNextTarget(SizeL(assignedAgents));
        pathsToSingleTarget(assignedAgents, target);
        for(tSigned32 j = 0; j != SizeL(assignedAgents); ++j)
        {
            tSigned32 i = assignedAgents[j];
            if(!_agentPaths[i])
            {
                order->notifyAgentCompletedOrAbandonedWaypoint();
            }
        }
    }

    // advance agents
    std::vector<float> speeds(_agents.size(), agentSpeed);
    {
        cVectorReceiver<iAgent*> agentsReceiver;
        agentsReceiver.vector.reserve(_agents.size());
        for(i = 0; i != SizeL(_agents); ++i)
        {
            if(_agentPaths[i] == 0)
            {
                continue;
            }
            tUnsigned32 pathLength = _agentPaths[i]->getLength();
            agentsReceiver.vector.clear();
            _mesh->getAllAgentsOverlapped(_agentShape, _avoidContext, _avoidAgents[i]->getPosition(), agentsReceiver);
            for(tSigned32 j = 0; j != agentsReceiver.vector.size(); ++j)
            {
                iAgent* overlappedAgent = agentsReceiver.vector[j];
                if(overlappedAgent == _agents[i])
                {
                    continue;
                }
                tSigned64 overlappedI = overlappedAgent->getUserData();
                //.... also check that the agents have same destination!
                if(_agentPaths[static_cast<size_t>(overlappedI)] != 0 && _agentPaths[static_cast<size_t>(overlappedI)]->getLength() < pathLength)
                {
                    speeds[i] = agentSpeed_Queuing;
                    break;
                }
            }
        }
    }
    for(i = 0; i != SizeL(_agents); ++i)
    {
        if(_agentPaths[i] == 0)
        {
            continue;
        }
        assertD(!_atRestContext->includes(_agents[i]));
        iCollisionInfo* info = _agents[i]->advanceAlongPath(_agentPaths[i], speeds[i], _atRestContext);
        _avoidAgents[i]->moveTo(_agents[i]->getPosition());
        if(info || _agentPaths[i]->size() == 1)
        {
            delete info;
            delete _agentPaths[i];
            _agentPaths[i] = 0;
            assertD(!_atRestContext->includes(_agents[i]));
            if(_mesh->testPointCollision(_agentShape, _atRestContext, _agents[i]->getPosition()))
            {
                cPosition target = _mesh->findClosestUnobstructedPosition(_agentShape, _atRestContext, _agents[i]->getPosition(), positionResolveDistance);
                if(target.cell != -1)
                {
                    _agentPaths[i] = _agents[i]->findShortestPathTo(_atRestContext, target);
                }
            }
            if(!_agentPaths[i])
            {
                _atRestContext->addAgent(_agents[i]);
                _avoidContext->removeAgent(_avoidAgents[i]);
                if(_agentMovementOrders[i] != -1)
                {
                    _movementOrders[_agentMovementOrders[i]]->notifyAgentCompletedOrAbandonedWaypoint();
                }
            }
        }
    }
}
