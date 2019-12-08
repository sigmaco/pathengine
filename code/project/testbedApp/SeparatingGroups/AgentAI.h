
//**********************************************************************
//
// Copyright (c) 2012
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include <vector>

class iMesh;
class iShape;
class iCollisionContext;
class iAgent;
class iPath;
class cPosition;
class cMovementOrder;

class cAgentAI
{
    iMesh* _mesh;
    iShape* _agentShape;
    iShape* _avoidShape;
    iCollisionContext* _atRestContext;
    iCollisionContext* _avoidContext;
    std::vector<iAgent*> _agents;
    std::vector<iAgent*> _avoidAgents;
    std::vector<iPath*> _agentPaths;
    std::vector<cMovementOrder*> _movementOrders;
    std::vector<tSigned32> _agentMovementOrders;

    void pathsToSingleTarget(const std::vector<tSigned32>& selectedAgents, const cPosition& targetPosition);

public:

    cAgentAI(iMesh* mesh, iShape* agentShape, iShape* avoidShape);
    ~cAgentAI();

    tSigned32
    numberOfAgents() const
    {
        return SizeL(_agents);
    }
    iAgent*
    getAgent(tSigned32 i)
    {
        return _agents[i];
    }
    // avoid agents are used for rendering, to highlight selected agents
    iAgent*
    getHighlightAgent(tSigned32 i)
    {
        return _avoidAgents[i];
    }
    tSigned32
    agentGroupIndex(tSigned32 i) const
    {
        return _agentMovementOrders[i];
    }

    void stopOrder(const std::vector<tSigned32>& selectedAgents);
    void moveOrder(const std::vector<tSigned32>& selectedAgents, const cPosition& targetPosition);
    void moveOrder(const std::vector<tSigned32>& selectedAgents, const std::vector<cPosition>& waypoints, bool repeat);
    void update();
};
