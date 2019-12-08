
#include "base/Header.h"
#include "sampleShared/Error.h"
#include "sampleShared/LoadBinary.h"
#include "base/MutexLock.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "platform_common/Thread.h"
#include "platform_common/Mutex.h"
#include "externalAPI/i_pathengine.h"
#include "externalAPI/i_testbed.h"
#include <vector>
#include <string>
#include <sstream>

namespace
{

class cMeshLoadingQueue
{
    tSigned32 _meshIterations;
    cMutex* _queueMutexes;
    iMesh** _queueMeshes;
    tSigned32 _nextIndex;

    cMutex _stateMutex;
    tSigned32 _numberLoaded;
    bool _ready;

public:

    // initialisation to be performed before foreground thread creation
    cMeshLoadingQueue(tSigned32 meshIterations);

    // called by background thread to load meshes and place on queue
    void loadMeshesAndPlaceOnQueue(
            iPathEngine* pathEngine,
            const char* meshName,
            const std::vector<iShape*>& baseObstacleShapes,
            iShape* agentShape, const char** pathfindPreprocessAttributes
            );

    // called by foreground thread before getting meshes from queue
    // (required for background thread to lock first queue mutex)
    void waitUntilReady();

    // called by foreground thread to obtain meshes from the queue
    // returns zero when queue is empty
    // the calling code takes responsibility for deleting the mesh
    iMesh* getNext();

    // called by the foreground thread just for printing in the testbed
    tSigned32 getNumberLoaded() const;
    tSigned32 getQueuePosition() const
    {
        return _nextIndex;
    }

    // destruction after foreground thread is completed
    ~cMeshLoadingQueue();
};

class cThread : public iThread
{
    iPathEngine* _pathEngine;
    cMeshLoadingQueue& _queue;
    iShape* _agentShape;
    const std::vector<iShape*>& _baseObstacleShapes;
    const char** _pathfindPreprocessAttributes;
    
public:

    cThread(
        iPathEngine* pathEngine, 
        cMeshLoadingQueue& queue,
        iShape* agentShape,
        const std::vector<iShape*>& baseObstacleShapes,
        const char** pathfindPreprocessAttributes
        ) :
     _pathEngine(pathEngine),
     _queue(queue),
     _agentShape(agentShape),
     _baseObstacleShapes(baseObstacleShapes),
     _pathfindPreprocessAttributes(pathfindPreprocessAttributes)
    {
    }
    
    void run();
};

} // end anonymous namespace

iMesh*
GenerateRandomMesh(iPathEngine* pathEngine, const char* meshName, const std::vector<iShape*>& baseObstacleShapes)
{
    iMesh* mesh;
    {
        std::string meshPath = "../resource/meshes/";
        meshPath += meshName;
        meshPath += ".tok";
        std::vector<char> buffer;
        LoadBinary(meshPath.c_str(), buffer);
        assertR(!buffer.empty());
        mesh = pathEngine->loadMeshFromBuffer("tok", &buffer[0], SizeL(buffer), 0);
    }
    iCollisionContext* context = mesh->newContext();
    tSigned32 i;
    for(i = 0; i < static_cast<tSigned32>(baseObstacleShapes.size()); i++)
    {
        cPosition randomPosition;
        do
        {
            randomPosition = mesh->generateRandomPosition();
        }
        while(randomPosition.cell == -1);
        iAgent* agent = mesh->placeAgent(baseObstacleShapes[i], randomPosition);
        context->addAgent(agent);
    }
    mesh->burnContextIntoMesh(context);
    return mesh;
}

cMeshLoadingQueue::cMeshLoadingQueue(tSigned32 meshIterations)
{
    _meshIterations = meshIterations;
    _queueMutexes = new cMutex[_meshIterations];
    _queueMeshes = new iMesh*[_meshIterations];
    _nextIndex = 0;
    _numberLoaded = 0;
    _ready = false;
}

void
cMeshLoadingQueue::loadMeshesAndPlaceOnQueue(
            iPathEngine* pathEngine,
            const char* meshName, const std::vector<iShape*>& baseObstacleShapes,
            iShape* agentShape, const char** unobstructedSpaceAttributes
            )
{
    assertD(_meshIterations);
    _queueMutexes[0].lock();
    {
        cMutexLock lock(_stateMutex);
        _ready = true;
    }
    for(tSigned32 i = 0; i < _meshIterations; ++i)
    {
        _queueMeshes[i] = GenerateRandomMesh(pathEngine, meshName, baseObstacleShapes);
        _queueMeshes[i]->generateUnobstructedSpaceFor(agentShape, true, unobstructedSpaceAttributes);
        _queueMeshes[i]->generatePathfindPreprocessFor(agentShape, 0);
        if(i + 1 < _meshIterations)
        {
            _queueMutexes[i + 1].lock();
        }
        _queueMutexes[i].unlock();

        cMutexLock lock(_stateMutex);
        _numberLoaded++;
    }
}

void
cMeshLoadingQueue::waitUntilReady()
{
  // one busy wait, just to start queue synchronisation
  // (this is necessary because on some platforms mutex lock and unlock must be in same thread)
    bool ready;
    do
    {
        cMutexLock lock(_stateMutex);
        ready = _ready;
    }
    while(!ready);
}

iMesh*
cMeshLoadingQueue::getNext()
{
    assertR(_ready); // waitUntilReady() must be called before starting to query queue
    if(_nextIndex == _meshIterations)
    {
        return 0;
    }
    // the critical section is locked here as a way of waiting for the next queue item to be ready
    _queueMutexes[_nextIndex].lock();
    _queueMutexes[_nextIndex].unlock();
    return _queueMeshes[_nextIndex++];
}

tSigned32
cMeshLoadingQueue::getNumberLoaded() const
{
    cMutexLock lock(_stateMutex);
    return _numberLoaded;
}

cMeshLoadingQueue::~cMeshLoadingQueue()
{
    assertD(_nextIndex == _meshIterations);
    delete [] _queueMutexes;
    delete [] _queueMeshes;
}

void DrawContext(iTestBed* testbed, iCollisionContext* context)
{
    for(tSigned32 i = 0; i < context->getNumberOfAgents(); i++)
    {
        iAgent* agent = context->getAgent(i);
        testbed->drawAgent(agent, 40);
    }
}

void
DoPathIterations(
        iTestBed* testBed,
        iShape* agentShape,
        tSigned32 pathIterations,
        const cMeshLoadingQueue& queue,
        iMesh* mesh, iCollisionContext* context
        )
{
    do
    {
        testBed->setColourRGB(0.0f,0.0f,0.85f);
        testBed->drawMesh();
        testBed->setColour("blue");
        testBed->drawMeshEdges();
        testBed->setColour("red");
        testBed->drawBurntInObstacles();
        testBed->setColour("orange");
        DrawContext(testBed, context);

        {
            std::ostringstream os;
            os << "meshes dispatched = " << queue.getQueuePosition();
            testBed->printTextLine(10, os.str().c_str());
        }
        {
            std::ostringstream os;
            os << "meshes loaded = " << queue.getNumberLoaded();
            testBed->printTextLine(10, os.str().c_str());
        }

        for(tSigned32 i = 0; i < 10; i++)
        {
            if(!pathIterations)
            {
                break;
            }
            cPosition start;
            do
            {
                start = mesh->generateRandomPosition();
            }
            while(start.cell == -1 || mesh->testPointCollision(agentShape, context, start));

            cPosition goal;
            do
            {
                goal = mesh->generateRandomPosition();
            }
            while(goal.cell == -1);

            iPath* path = mesh->findShortestPath(agentShape, context, start, goal);
            testBed->setColour("green");
            testBed->drawPath(path);
            delete path;

            pathIterations--;
        }

        testBed->update_ExitIfRequested();
    }
    while(pathIterations);
}

iCollisionContext*
InitContext(iMesh* mesh, const std::vector<iShape*>& shapes, std::vector<iAgent*>& agents)
{
    iCollisionContext* result = mesh->newContext();
    cPosition randomPosition;
    do
    {
        randomPosition = mesh->generateRandomPosition();
    }
    while(randomPosition.cell == -1);
    size_t i;
    for(i = 0; i < shapes.size(); i++)
    {
        iAgent* placed = mesh->placeAgent(shapes[i], randomPosition);
        result->addAgent(placed);
        agents[i] = placed;
    }
    return result;
}
void
DeleteContextAndAgents(iCollisionContext* context)
{
    while(context->getNumberOfAgents())
    {
        iAgent* agent = context->getAgent(0);
        delete agent;
    }
    delete context;
}

void
DoDynamicObstacleIterations(
        iTestBed* testBed,
        iShape* agentShape,
        const std::vector<iShape*>& dynamicObstacleShapes,
        tSigned32 dynamicObstacleIterations,
        tSigned32 pathIterations,
        const cMeshLoadingQueue& queue,
        iMesh* mesh
        )
{
    tSigned32 dynamicObstacles = SizeL(dynamicObstacleShapes);
    std::vector<iAgent*> agents(dynamicObstacles);
    iCollisionContext* context = InitContext(mesh, dynamicObstacleShapes, agents);
    for(tSigned32 i = 0; i < dynamicObstacleIterations; i++)
    {
        for(tSigned32 j = 0; j < dynamicObstacles; j++)
        {
            cPosition randomPosition;
            do
            {
                randomPosition = mesh->generateRandomPosition();
            }
            while(randomPosition.cell == -1);
            agents[j]->moveTo(randomPosition);
        }
        DoPathIterations(testBed, agentShape, pathIterations, queue, mesh, context);
    }
    DeleteContextAndAgents(context);
}

void
cThread::run()
{
    _queue.loadMeshesAndPlaceOnQueue(_pathEngine, "mesh1", _baseObstacleShapes, _agentShape, _pathfindPreprocessAttributes);
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

    iShape* shape_Square20;
    {
        tSigned32 array[]=
        {
            -20, 20,
            20, 20,
            20, -20,
            -20, -20,
        };
        shape_Square20 = pathEngine->newShape(array, sizeof(array) / sizeof(*array));
    }
    iShape* shape_Octagon8_20;
    {
        tSigned32 array[]=
        {
            8,20,
            20,8,
            20,-8,
            8,-20,
            -8,-20,
            -20,-8,
            -20,8,
            -8,20,
        };
        shape_Octagon8_20 = pathEngine->newShape(array, sizeof(array) / sizeof(*array));
    }
    iShape* shape_Swarmer;
    {
        tSigned32 array[]=
        {
            0,16,
            14,-8,
            -14,-8,
        };
        shape_Swarmer = pathEngine->newShape(array, sizeof(array) / sizeof(*array));
    }
    iShape* shape_Square60;
    {
        tSigned32 array[]=
        {
            -60,60,
            60,60,
            60,-60,
            -60,-60,
        };
        shape_Square60 = pathEngine->newShape(array, sizeof(array) / sizeof(*array));
    }

    const char* attributes[3];
    attributes[0] = "splitWithCircumferenceBelow";
    attributes[1] = "2000";
    attributes[2] = 0;

    std::vector<iShape*> baseObstacles;
    baseObstacles.push_back(shape_Square60);
    baseObstacles.push_back(shape_Square60);
    baseObstacles.push_back(shape_Square60);
    baseObstacles.push_back(shape_Square60);
    baseObstacles.push_back(shape_Square60);
    baseObstacles.push_back(shape_Swarmer);
    baseObstacles.push_back(shape_Swarmer);
    baseObstacles.push_back(shape_Swarmer);
    baseObstacles.push_back(shape_Swarmer);
    baseObstacles.push_back(shape_Swarmer);
    baseObstacles.push_back(shape_Swarmer);
    baseObstacles.push_back(shape_Square20);
    baseObstacles.push_back(shape_Square20);
    baseObstacles.push_back(shape_Octagon8_20);
    baseObstacles.push_back(shape_Octagon8_20);
    baseObstacles.push_back(shape_Octagon8_20);
    baseObstacles.push_back(shape_Octagon8_20);
    std::vector<iShape*> dynamicObstacles;
    dynamicObstacles.push_back(shape_Square60);
    dynamicObstacles.push_back(shape_Swarmer);
    dynamicObstacles.push_back(shape_Octagon8_20);

    cMeshLoadingQueue queue(100);

    iShape* agentShape = shape_Swarmer;

    cThread thread(
            pathEngine,
            queue,
            agentShape,
            baseObstacles, attributes
            );
            
    tThreadHandle handle = StartThread(thread);

    queue.waitUntilReady();

    bool isFirst = true;
    iMesh* mesh;
    while(mesh = queue.getNext())
    {
        testBed->setMesh(mesh);
        if(isFirst)
        {
            testBed->zoomExtents();
            isFirst = false;
        }
        DoDynamicObstacleIterations(
                testBed,
                agentShape,
                dynamicObstacles,
                5, // dynamic obstacle iterations
                50, // path iterations
                queue,
                mesh
                );
        delete mesh;
    }

    JoinThread(handle);
}
