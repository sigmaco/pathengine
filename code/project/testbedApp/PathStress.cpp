
#include "base/Header.h"
#include "sampleShared/Error.h"
#include "sampleShared/GenerateAndSaveRandomSeed.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "externalAPI/i_pathengine.h"
#include "externalAPI/i_testbed.h"
#include <vector>
#include <string>

void DoPathStress( iTestBed* testbed,
                   iMesh* mesh,
                   iShape* agentShape,
                   iCollisionContext* context,
                   const cPosition& start, const cPosition& goal)
{
    iPath* path = mesh->findShortestPath(agentShape, context, start, goal);
    if(mesh->testPointCollision(agentShape, context, goal))
    {
        assertR(path == 0);
        assertR(mesh->testLineCollision(agentShape, context, start, goal) == true);
        {
            tSigned32 endCell;
            bool collided = mesh->testLineCollision_XY(agentShape, context, start, goal.x, goal.y, endCell);
            assertR(collided || endCell != goal.cell);
            tSigned32 endCell2;
            iCollisionInfo* info = mesh->firstCollision(agentShape, context, start, goal.x, goal.y, endCell2);
            assertR(collided == (info != 0));
            if(!collided)
            {
                assertR(endCell2 == endCell);
            }
            delete info;
        }
        return;
    }

    if(!path)
    {
        return;
    }

    testbed->setColour("green");
    testbed->drawPath(path);

    tSigned32 i;
    for(i = 0; i + 1 < path->size(); i++)
    {
        cPosition segmentStart = path->position(i);
        cPosition segmentEnd = path->position(i + 1);
        assertR(segmentStart != segmentEnd);
        assertR(mesh->testLineCollision(agentShape, context, segmentStart, segmentEnd) == false);
        assertR(mesh->testLineCollision(agentShape, context, segmentEnd, segmentStart) == false);
        {
            tSigned32 endCell;
            bool collided = mesh->testLineCollision_XY(agentShape, context, segmentStart, segmentEnd.x, segmentEnd.y, endCell);
            assertR(!collided && endCell == segmentEnd.cell);
            iCollisionInfo* info = mesh->firstCollision(agentShape, context, segmentStart, segmentEnd.x, segmentEnd.y, endCell);
            assertR(info == 0 && endCell == segmentEnd.cell);
        }
    }
    delete path;
}

void DrawContext(iTestBed* testbed, iCollisionContext* context)
{
    tSigned32 i;
    for(i = 0; i < context->getNumberOfAgents(); i++)
    {
        iAgent* agent = context->getAgent(i);
        testbed->drawAgent(agent, 40);
    }
}

void DoPathIterations(
                   iTestBed* testbed,
                   iMesh* mesh,
                   iShape* agentShape,
                   iCollisionContext* baseObstaclesContext,
                   iCollisionContext* context,
                   tSigned32 pathIterations)
{
    do
    {
        testbed->setColourRGB(0.0f,0.0f,0.85f);
        testbed->drawMesh();
        testbed->setColour("blue");
        testbed->drawMeshEdges();
        testbed->setColour("red");
        DrawContext(testbed, baseObstaclesContext);
        testbed->setColour("orange");
        DrawContext(testbed, context);

        tSigned32 i;
        for(i = 0; i < 10; i++)
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

            DoPathStress(testbed, mesh, agentShape, context, start, goal);
            pathIterations--;
        }

        testbed->update_ExitIfRequested();
    }
    while(pathIterations);
}

iCollisionContext* InitContext(iMesh* mesh, const std::vector<iShape*>& shapes, std::vector<iAgent*>& agents)
{
    iCollisionContext* result = mesh->newContext();
    cPosition randomPosition;
    do
    {
        randomPosition = mesh->generateRandomPosition();
    }
    while(randomPosition.cell == -1);
    for(tSigned32 i = 0; i < SizeL(shapes); i++)
    {
        iAgent* placed = mesh->placeAgent(shapes[i], randomPosition);
        result->addAgent(placed);
        agents[i] = placed;
    }
    return result;
}
void DeleteContextAndAgents(iCollisionContext* context)
{
    while(context->getNumberOfAgents())
    {
        iAgent* agent = context->getAgent(0);
        delete agent;
    }
    delete context;
}

void DoDynamicObstacleIterations(
                   iTestBed* testbed,
                   iMesh* mesh,
                   iShape* agentShape,
                   iCollisionContext* baseObstaclesContext,
                   const std::vector<iShape*>& dynamicObstacleShapes,
                   tSigned32 dynamicObstacleIterations,
                   tSigned32 pathIterations)
{
    tSigned32 dynamicObstacles = SizeL(dynamicObstacleShapes);
    std::vector<cPosition> positions(dynamicObstacles);
    std::vector<iAgent*> agents(dynamicObstacles);
    iCollisionContext* context = InitContext(mesh, dynamicObstacleShapes, agents);
    tSigned32 i;
    for(i = 0; i < dynamicObstacleIterations; i++)
    {
        tSigned32 j;
        for(j = 0; j < dynamicObstacles; j++)
        {
            cPosition randomPosition;
            do
            {
                randomPosition = mesh->generateRandomPosition();
            }
            while(randomPosition.cell == -1);
            positions[j] = randomPosition; // the positions are recorded for debugging purposes
            agents[j]->moveTo(randomPosition);
        }

        DoPathIterations(testbed, mesh, agentShape, baseObstaclesContext, context, pathIterations);
    }
    DeleteContextAndAgents(context);
}

void RunStressTest(iPathEngine* pathengine, iTestBed* testbed,
                   const std::string& meshName,
                   iShape* agentShape,
                   const std::vector<iShape*>& baseObstacleShapes,
                   const std::vector<iShape*>& dynamicObstacleShapes,
                   tSigned32 baseObstacleIterations,
                   tSigned32 dynamicObstacleIterations,
                   tSigned32 pathIterations,
                   const char** unobstructedSpaceAttributes,
                   const char** loadOptions
                   )
{
    char* meshBuffer;
    tUnsigned32 meshBufferSize;
    {
        std::string meshPath = "../resource/meshes/" + meshName + ".tok";
        meshBuffer = testbed->loadBinary(meshPath.c_str(), meshBufferSize);
    }

    tSigned32 baseObstacles = SizeL(baseObstacleShapes);
    std::vector<cPosition> positions(baseObstacles);
    tSigned32 i;
    for(i = 0; i < baseObstacleIterations; i++)
    {
        iMesh* mesh = pathengine->loadMeshFromBuffer("tok", meshBuffer, meshBufferSize, loadOptions);

        iCollisionContext* context = mesh->newContext();
        tSigned32 j;
        for(j = 0; j < baseObstacles; j++)
        {
            cPosition randomPosition;
            do
            {
                randomPosition = mesh->generateRandomPosition();
            }
            while(randomPosition.cell == -1);
            iAgent* agent = mesh->placeAgent(baseObstacleShapes[j], randomPosition);
            context->addAgent(agent);
            positions[j] = randomPosition; // recorded for debugging purposes
        }

        mesh->burnContextIntoMesh(context);
        mesh->generateUnobstructedSpaceFor(agentShape, true, unobstructedSpaceAttributes);
        mesh->generatePathfindPreprocessFor(agentShape, 0);

        testbed->setMesh(mesh);
        if(i == 0)
        {
            testbed->zoomExtents();
        }

        DoDynamicObstacleIterations(testbed, mesh, agentShape, context, dynamicObstacleShapes, dynamicObstacleIterations, pathIterations);

        delete mesh;
    }

    testbed->freeBuffer(meshBuffer);
}


void
TestbedApplicationMain(iPathEngine *pathengine, iTestBed *testbed)
{
// check if interfaces are compatible with the headers used for compilation
    if(testbed->getInterfaceMajorVersion()!=TESTBED_INTERFACE_MAJOR_VERSION
        ||
        testbed->getInterfaceMinorVersion()<TESTBED_INTERFACE_MINOR_VERSION)
    {
        testbed->reportError("Fatal","Testbed version is incompatible with headers used for compilation.");
        return;
    }
    if(pathengine->getInterfaceMajorVersion()!=PATHENGINE_INTERFACE_MAJOR_VERSION
        ||
        pathengine->getInterfaceMinorVersion()<PATHENGINE_INTERFACE_MINOR_VERSION)
    {
        testbed->reportError("Fatal","Pathengine version is incompatible with headers used for compilation.");
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
        shape_Square20 = pathengine->newShape(array, sizeof(array) / sizeof(*array));
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
        shape_Octagon8_20 = pathengine->newShape(array, sizeof(array) / sizeof(*array));
    }
    iShape* shape_Swarmer;
    {
        tSigned32 array[]=
        {
            0,16,
            14,-8,
            -14,-8,
        };
        shape_Swarmer = pathengine->newShape(array, sizeof(array) / sizeof(*array));
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
        shape_Square60 = pathengine->newShape(array, sizeof(array) / sizeof(*array));
    }

    const char* attributes_SmallConvexOpt[3];
    attributes_SmallConvexOpt[0] = "splitWithCircumferenceBelow";
    attributes_SmallConvexOpt[1] = "2000";
    attributes_SmallConvexOpt[2] = 0;

    const char* attributes_UnobstructedSpaceOpt[3];
    attributes_UnobstructedSpaceOpt[0] = "optimiseWithRange";
    attributes_UnobstructedSpaceOpt[1] = "15";
    attributes_UnobstructedSpaceOpt[2] = 0;

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

    // ****
    // A random seed will be generated and saved to the file 'savedRandomSeed.txt',
    // in the bin directory of the release.
    // If a assertR fails, or if PathEngine crashes or hangs, please report this value.

    while(1)
    {
        tUnsigned32 seed = GenerateAndSaveRandomSeed();
        pathengine->setRandomSeed(seed);

        RunStressTest(pathengine, testbed, "mesh1", shape_Swarmer, baseObstacles, dynamicObstacles,
                        10, // baseObstacleIterations
                        5, // dynamicObstacleIterations
                        100, // pathIterations
                        0, 0
                        );
        RunStressTest(pathengine, testbed, "mesh1", shape_Swarmer, baseObstacles, dynamicObstacles,
                        10, // baseObstacleIterations
                        5, // dynamicObstacleIterations
                        100, // pathIterations
                        attributes_SmallConvexOpt, 0
                        );
        RunStressTest(pathengine, testbed, "mesh1", shape_Octagon8_20, baseObstacles, dynamicObstacles,
                        10, // baseObstacleIterations
                        5, // dynamicObstacleIterations
                        100, // pathIterations
                        0, 0
                        );
        RunStressTest(pathengine, testbed, "mesh1", shape_Octagon8_20, baseObstacles, dynamicObstacles,
                        10, // baseObstacleIterations
                        5, // dynamicObstacleIterations
                        100, // pathIterations
                        attributes_SmallConvexOpt, 0
                        );
        RunStressTest(pathengine, testbed, "mesh1", shape_Square20, baseObstacles, dynamicObstacles,
                        10, // baseObstacleIterations
                        5, // dynamicObstacleIterations
                        100, // pathIterations
                        attributes_UnobstructedSpaceOpt, 0
                        );
    }
}
