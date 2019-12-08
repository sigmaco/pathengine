#include "base/Header.h"
#include "project/testbedApp/ThreadPerMesh/MeshAndAgents.h"
#include "base/MutexLock.h"
#include "platform_common/Thread.h"
#include "externalAPI/i_testbed.h"

void
cMeshAndAgents::generatePreprocessIfNotPresent()
{
    if(!_mesh->shapeCanPathfind(_agentShape, 0))
    {
        const char* options[3];
        options[0] = "splitWithCircumferenceBelow";
        options[1] = "2000";
        options[2] = 0;
        //.... load preprocess from persistence, normally, for best performance
        _mesh->generateUnobstructedSpaceFor(_agentShape, true, options);
        _mesh->generateConnectedRegionPreprocessFor(_agentShape, 0);
        _mesh->generatePathfindPreprocessFor(_agentShape, 0);
    }
}

void
cMeshAndAgents::doAddAgent()
{
    generatePreprocessIfNotPresent();
    cPosition p;
    do
    {
        p = _mesh->generateRandomPosition();
    }
    while(_mesh->testPointCollision(_agentShape, 0, p));
    _agents.push_back(_mesh->placeAgent(_agentShape, p));
    _paths.push_back(0);
}

void
cMeshAndAgents::updateAgent(tSigned32 i)
{
    if(_paths[i])
    {
        iCollisionInfo* info = _agents[i]->advanceAlongPath(_paths[i], 12.f, 0);
        assertD(info == 0);
        if(_paths[i]->size() < 2)
        {
            delete _paths[i];
            _paths[i] = 0;
        }
        return;
    }
// attempt to generate a new path
    cPosition p = _mesh->generateRandomPosition();
    if(_mesh->getConnectedRegionFor(_agentShape, p) != _mesh->getConnectedRegionFor(_agentShape, _agents[i]->getPosition()))
    {
        return;
    }
    {
        cMutexLock lock(_queryCountMutex);
        ++_queryCount;
    }
    _paths[i] = _agents[i]->findShortestPathTo(0, p);
}

cMeshAndAgents::cMeshAndAgents(
        iPathEngine* pathEngine, iShape* agentShape,
        char* meshBuffer, tUnsigned32 meshBufferSize
        )
{
    _agentShape = agentShape;
    _mesh = pathEngine->loadMeshFromBuffer("tok", meshBuffer, meshBufferSize, 0);
    _queryCount = 0;
    _agentsToAdd = 0;
    _terminate = false;
}
cMeshAndAgents::~cMeshAndAgents()
{
    _mesh->destroy(); // note that PathEngine also cleans up any outstanding agents or paths associated with the mesh, here
}

void
cMeshAndAgents::run()
{
    bool terminate;
    do
    {
        {
            cMutexLock lock(_positionsMutex);
            _positionsBeforeUpdate.resize(_agents.size());
            for(tSigned32 i = 0; i != _agents.size(); ++i)
            {
                _positionsBeforeUpdate[i] = _agents[i]->getPosition();
            }
        }

        {
            tSigned32 agentsToAdd;
            {
                cMutexLock lock(_agentAddMutex);
                agentsToAdd = _agentsToAdd;
                _agentsToAdd = 0;
            }
            for(tSigned32 i = 0; i != agentsToAdd; ++i)
            {
                doAddAgent();
            }
        }

        for(tSigned32 i = 0; i != _agents.size(); ++i)
        {
            updateAgent(i);
        }

        Sleep_Milliseconds(1);

        cMutexLock lock(_terminateMutex);
        terminate = _terminate;
    }
    while(!terminate);
}

void
cMeshAndAgents::addAgent()
{
    cMutexLock lock(_agentAddMutex);
    ++_agentsToAdd;
}

void
cMeshAndAgents::render(iTestBed* testBed, tSigned32 meshSlot, bool initCamera) const
{
    testBed->setActiveMeshSlot(meshSlot);
    testBed->setMeshAtSlot(meshSlot, _mesh);
    if(initCamera)
    {
        testBed->zoomExtents();
    }
    testBed->setColourRGB(0.0f,0.0f,0.85f);
    testBed->drawMesh();
    testBed->setColour("blue");
    testBed->drawMeshEdges();
    std::vector<cPosition> positions;
    {
        cMutexLock lock(_positionsMutex);
        positions = _positionsBeforeUpdate;
    }
    testBed->setColour("orange");
    for(tSigned32 i = 0; i != positions.size(); ++i)
    {
        testBed->drawShape(_agentShape, positions[i], 120);
    }
}

tSigned32
cMeshAndAgents::getQueryCount() const
{
    cMutexLock lock(_queryCountMutex);
    return _queryCount;
}

void
cMeshAndAgents::terminate()
{
    cMutexLock lock(_terminateMutex);
    _terminate = true;
}
