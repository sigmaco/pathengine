//**********************************************************************
//
// Copyright (c) 2007
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include "base/Header.h"
#include "project/testbedApp/SemiDynamicObstacles/DynamicAgentManager.h"
#include "externalAPI/i_testbed.h"
#include "externalAPI/i_pathengine.h"
#include <stdlib.h>
cDynamicAgentManager::cDynamicAgentManager(iTestBed* testBed, iMesh* mesh, iShape* agentShape, const iCollisionContext* context, tSigned32 numberOfAgents) :
 _testBed(testBed),
 _mesh(mesh),
 _agentShape(agentShape),
 _agents(numberOfAgents),
 _paths(numberOfAgents),
 _regions(numberOfAgents, -1)
{
    assertD(mesh->shapeCanCollide(agentShape));
    tSigned32 i;
    for(i = 0; i != numberOfAgents; ++i)
    {
        cPosition p;
        do
        {
            p = mesh->generateRandomPosition();
        }
        while(mesh->testPointCollision(agentShape, context, p));
        _agents[i] = mesh->placeAgent(agentShape, p);
        do
        {
            p = mesh->generateRandomPosition();
        }
        while(mesh->testPointCollision(agentShape, context, p));
        _paths[i] = _agents[i]->findShortestPathTo(context, p);
    }
    _shapeRenderPreprocess = _testBed->preprocessShapeWithHeight(_agentShape, 60);
}
cDynamicAgentManager::~cDynamicAgentManager()
{
    tSigned32 i;
    for(i = 0; i != SizeL(_agents); ++i)
    {
        delete _agents[i];
        delete _paths[i];
    }
    _testBed->deleteRenderPreprocess(_shapeRenderPreprocess);
}

void
cDynamicAgentManager::moveAlongPaths(const iCollisionContext* context)
{
    tSigned32 i;
    for(i = 0; i != SizeL(_agents); ++i)
    {
        if(_paths[i])
        {
            iCollisionInfo* ci = _agents[i]->advanceAlongPath(_paths[i], 40.f, context);
            if(ci)
            {
                delete ci;
                delete _paths[i];
                _paths[i] = 0;
            }
            else
            {
                if(_paths[i]->size() == 1)
                {
                    delete _paths[i];
                    _paths[i] = 0;
                }
            }
        }
    }
}

void
cDynamicAgentManager::rePath(const iObstacleSet* preprocessedSet, const iCollisionContext* context, tSigned32 maxQueries)
{
    std::vector<tSigned32> toRepath;
    tSigned32 i;
    for(i = 0; i != SizeL(_agents); ++i)
    {
        if(_paths[i] == 0)
        {
            toRepath.push_back(i);
        }
    }
    tSigned32 numberToUpdate = maxQueries;
    if(numberToUpdate > SizeL(toRepath))
    {
        numberToUpdate = SizeL(toRepath);
    }
    for(i = 0; i != numberToUpdate; ++i)
    {
        tSigned32 r = rand() % SizeL(toRepath);
        tSigned32 selected = toRepath[r];
        tSigned32 currentRegion = preprocessedSet->getConnectedRegionForAgent(_agents[selected]);
        cPosition p;
        tSigned32 j;
        for(j = 0; j != 40; ++j)
        {
            p = _mesh->generateRandomPosition();
            if(preprocessedSet->getConnectedRegionFor(_agentShape, p) != currentRegion)
            {
                continue;
            }
            _paths[selected] = _agents[selected]->findShortestPathTo(context, p);
            break;
        }
        // swap this agent out so we dont repath the same agent multiple times
        toRepath[r] = toRepath.back();
        toRepath.pop_back();
    }
}

void
cDynamicAgentManager::assignRegions(const iObstacleSet* preprocessedSet)
{
    tSigned32 i;
    for(i = 0; i != SizeL(_agents); ++i)
    {
        _regions[i] = preprocessedSet->getConnectedRegionForAgent(_agents[i]);
    }
}

void
cDynamicAgentManager::renderAgents() const
{
    tSigned32 i;
    for(i = 0; i != SizeL(_agents); ++i)
    {
        _testBed->drawPreprocessedGeometryAtPosition(_shapeRenderPreprocess, _agents[i]->getPosition());
    }
}
void
cDynamicAgentManager::renderAgents_ColouredByRegion() const
{
    const char* colours[] = {"green", "orange", "yellow", "lightpurple", "pink", "lightblue"};
    tSigned32 numberOfColours = sizeof(colours) / sizeof(*colours);
    tSigned32 i;
    for(i = 0; i != SizeL(_agents); ++i)
    {
        if(_regions[i] == -1)
        {
            _testBed->setColour("red");
        }
        else
        {
            _testBed->setColour(colours[_regions[i] % numberOfColours]);
        }
        _testBed->drawPreprocessedGeometryAtPosition(_shapeRenderPreprocess, _agents[i]->getPosition());
    }
}
void
cDynamicAgentManager::renderPaths() const
{
    tSigned32 i;
    for(i = 0; i != SizeL(_paths); ++i)
    {
        _testBed->drawPath(_paths[i]);
    }
}
