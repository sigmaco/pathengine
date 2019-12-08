
#pragma once

#include "externalAPI/i_pathengine.h"
#include "platform_common/Mutex.h"
#include <vector>

class iTestBed;

class cMeshAndAgents
{
    iShape* _agentShape;
    iMesh* _mesh;
    std::vector<iAgent*> _agents;
    std::vector<iPath*> _paths;
    std::vector<cPosition> _positionsBeforeUpdate;
    tSigned32 _queryCount;
    tSigned32 _agentsToAdd;
    bool _terminate;
    mutable cMutex _positionsMutex;
    mutable cMutex _queryCountMutex;
    mutable cMutex _agentAddMutex;
    mutable cMutex _terminateMutex;

    void generatePreprocessIfNotPresent();
    void doAddAgent();
    void updateAgent(tSigned32 i);

public:

    cMeshAndAgents(
            iPathEngine* pathEngine, iShape* agentShape,
            char* meshBuffer, tUnsigned32 meshBufferSize
            );
    ~cMeshAndAgents();

// per thread execution logic, with update loop

    void run();

// called from main thread to affect execution

    void addAgent();

// called from main thread for rendering or stats

    void render(iTestBed* testBed, tSigned32 meshSlot, bool initCamera) const;
    tSigned32 getQueryCount() const;

// called from main thread to request termination

    void terminate();
};
