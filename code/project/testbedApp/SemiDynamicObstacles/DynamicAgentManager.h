//**********************************************************************
//
// Copyright (c) 2007
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef DYNAMIC_AGENT_MANAGER_INCLUDED
#define DYNAMIC_AGENT_MANAGER_INCLUDED

#include <vector>

class iTestBed;
class iAgent;
class iPath;
class iMesh;
class iShape;
class cRenderPreprocess;
class iObstacleSet;
class iCollisionContext;

class cDynamicAgentManager
{
    iTestBed* _testBed;
    iMesh* _mesh;
    iShape* _agentShape;
    cRenderPreprocess* _shapeRenderPreprocess;
    std::vector<iAgent*> _agents;
    std::vector<iPath*> _paths;
    std::vector<tSigned32> _regions;

public:

    cDynamicAgentManager(iTestBed* testBed, iMesh* mesh, iShape* agentShape, const iCollisionContext* context, tSigned32 numberOfAgents);
    ~cDynamicAgentManager();

    tSigned32 size()
    {
        return SizeL(_agents);
    }

    void moveAlongPaths(const iCollisionContext* context);
    void rePath(const iObstacleSet* preprocessedSet, const iCollisionContext* context, tSigned32 maxQueries);

    void assignRegions(const iObstacleSet* preprocessedSet);

    void renderAgents() const;
    void renderAgents_ColouredByRegion() const;
    void renderPaths() const;
};

#endif
