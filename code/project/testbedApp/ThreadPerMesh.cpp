#include "base/Header.h"
#include "project/testbedApp/ThreadPerMesh/MeshAndAgents.h"
#include "sampleShared/Error.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "platform_common/Thread.h"
#include "externalAPI/i_testbed.h"
#include "externalAPI/i_pathengine.h"
#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include <algorithm>

namespace
{
class cThread : public iThread
{
    cMeshAndAgents* _meshAndAgents;
public:
    cThread(cMeshAndAgents* meshAndAgents)
    {
        _meshAndAgents = meshAndAgents;
    }
    void
    run()
    {
        _meshAndAgents->run();
    }
};
} // anonymous namespace

static void
AddInstance(
        iPathEngine* pathEngine, iShape* agentShape, char* meshBuffer, tUnsigned32 meshBufferSize,
        std::vector<cMeshAndAgents*>& instances,
        std::vector<iThread*>& threads,
        std::vector<tThreadHandle>& threadHandles        
        )
{
    cMeshAndAgents* instance = new cMeshAndAgents(pathEngine, agentShape, meshBuffer, meshBufferSize);
    cThread* thread = new cThread(instance);
    instances.push_back(instance);
    threads.push_back(thread);
    threadHandles.push_back(StartThread(*thread));
}

void
TestbedApplicationMain(iPathEngine* pathEngine, iTestBed* testBed)
{
// check if interfaces are compatible with the headers used for compilation
    if(testBed->getInterfaceMajorVersion()!=TESTBED_INTERFACE_MAJOR_VERSION
        ||
        testBed->getInterfaceMinorVersion()<TESTBED_INTERFACE_MINOR_VERSION)
    {
        testBed->reportError("Fatal","Testbed version is incompatible with headers used for compilation.");
        return;
    }
    if(pathEngine->getInterfaceMajorVersion()!=PATHENGINE_INTERFACE_MAJOR_VERSION
        ||
        pathEngine->getInterfaceMinorVersion()<PATHENGINE_INTERFACE_MINOR_VERSION)
    {
        testBed->reportError("Fatal","Pathengine version is incompatible with headers used for compilation.");
        return;
    }

    iShape* agentShape;
    {
        tSigned32 array[]=
        {
            -30, 30,
            30, 30,
            30, -30,
            -30, -30,
        };
        agentShape = pathEngine->newShape(array, sizeof(array) / sizeof(*array));
    }

    std::vector<char*> meshBuffers;
    std::vector<tUnsigned32> meshBufferSizes;
    {
        tUnsigned32 size;
        meshBuffers.push_back(testBed->loadBinary("../resource/meshes/demo_level1.tok", size));
        meshBufferSizes.push_back(size);
    }
    {
        tUnsigned32 size;
        meshBuffers.push_back(testBed->loadBinary("../resource/meshes/demo_level2.tok", size));
        meshBufferSizes.push_back(size);
    }
    {
        tUnsigned32 size;
        meshBuffers.push_back(testBed->loadBinary("../resource/meshes/demo_biglevel.tok", size));
        meshBufferSizes.push_back(size);
    }
    {
        tUnsigned32 size;
        meshBuffers.push_back(testBed->loadBinary("../resource/meshes/demo_triviallevel.tok", size));
        meshBufferSizes.push_back(size);
    }


    std::vector<cMeshAndAgents*> instances;
    std::vector<iThread*> threads;
    std::vector<tThreadHandle> threadHandles;
    
    AddInstance(pathEngine, agentShape, meshBuffers[0], meshBufferSizes[0], instances, threads, threadHandles);
    AddInstance(pathEngine, agentShape, meshBuffers[1], meshBufferSizes[1], instances, threads, threadHandles);
    AddInstance(pathEngine, agentShape, meshBuffers[2], meshBufferSizes[2], instances, threads, threadHandles);
    AddInstance(pathEngine, agentShape, meshBuffers[3], meshBufferSizes[3], instances, threads, threadHandles);
    AddInstance(pathEngine, agentShape, meshBuffers[2], meshBufferSizes[2], instances, threads, threadHandles);

    for(tSigned32 i = 0; i != meshBuffers.size(); ++i)
    {
        testBed->freeBuffer(meshBuffers[i]);
    }

    tSigned32 currentInstance = 0;

    bool changedInstance = true;
    bool exitFlag = false;
    while(!exitFlag)
    {
        testBed->update(exitFlag);

        instances[currentInstance]->render(testBed, currentInstance, changedInstance);
        changedInstance = false;

        for(tSigned32 i = 0; i != SizeL(instances); ++i)
        {
            std::ostringstream oss;
            oss << "instance " << i << ": " << instances[i]->getQueryCount() << " queries";
            testBed->printTextLine(0, oss.str().c_str());
        }

    // receive and process messages for all keys pressed since last frame
        const char *keypressed;
        while(keypressed = testBed->receiveKeyMessage())
        {
            if(keypressed[0] != 'd') // is it a key down message?
                continue;

            switch(keypressed[1])
            {
            case '_':
                {
                    if(!strcmp("ESCAPE", keypressed + 2))
                    {
                        exitFlag = true;
                        break;
                    }
                    if(!strcmp("UP", keypressed + 2))
                    {
                        if(currentInstance + 1 < static_cast<tSigned32>(instances.size()))
                        {
                            ++currentInstance;
                            changedInstance = true;
                        }
                        break;
                    }
                    if(!strcmp("DOWN", keypressed + 2))
                    {
                        if(currentInstance)
                        {
                            --currentInstance;
                            changedInstance = true;
                        }
                        break;
                    }
                    break;
                }
            case 'A':
                instances[currentInstance]->addAgent();
                break;
            }
        }
    }

    for(tSigned32 i = 0; i != SizeL(instances); ++i)
    {
        instances[i]->terminate();
    }
    for(tSigned32 i = 0; i != SizeL(threads); ++i)
    {
        JoinThread(threadHandles[i]);
        delete threads[i];
    }
}
