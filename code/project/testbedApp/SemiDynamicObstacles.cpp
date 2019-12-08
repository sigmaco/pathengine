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
#include "project/testbedApp/SemiDynamicObstacles/DoubleBufferedObstacleSet.h"
#include "project/testbedApp/SemiDynamicObstacles/DynamicAgentManager.h"
#include "sampleShared/PartitionedTerrain.h"
#include "sampleShared/LoadBinary.h"
#include "sampleShared/Error.h"
#include "common/FileOutputStream.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "externalAPI/i_testbed.h"
#include "externalAPI/i_pathengine.h"
#include <string>
#include <string.h>
#include <vector>
#include <sstream>
#include <math.h>
#include <time.h>

//#define USER_CONTROLLED_AGENT

class cAverageTimeTracker
{
    std::vector<double> _times;
    tSigned32 _bufferPosition;
    bool _bufferFull;
public:
    cAverageTimeTracker(tSigned32 bufferSize) :
      _times(bufferSize),
      _bufferPosition(0),
      _bufferFull(false)
    {
        assertD(bufferSize > 0);
    }
    void addTiming(double value)
    {
        _times[_bufferPosition++] = value;
        if(_bufferPosition == SizeL(_times))
        {
            _bufferPosition = 0;
            _bufferFull = true;
        }
    }
    bool ready() const
    {
        return _bufferFull;
    }
    double get()
    {
        assertD(ready());
        double result = _times[0];
        tSigned32 i;
        for(i = 1; i != SizeL(_times); ++i)
        {
            result += _times[i];
        }
        return result /= static_cast<double>(SizeL(_times));
    }
};

static void
RotateShapeCoords(tSigned32 vertices, const double* vertexCoords, double rotateBy, std::vector<tSigned32>& result)
{
    double sinOf = sin(rotateBy);
    double cosOf = cos(rotateBy);
    result.resize(vertices * 2);
    tSigned32 i;
    for(i = 0; i != vertices; ++i)
    {
        double x = vertexCoords[i * 2];
        double y = vertexCoords[i * 2 + 1];
        double rotatedX = sinOf * y + cosOf * x;
        double rotatedY = cosOf * y - sinOf * x;
        result[i * 2] = static_cast<tSigned32>(rotatedX);
        result[i * 2 + 1] = static_cast<tSigned32>(rotatedY);
    }
}

static void
PreGenerateRotationShapes(
        iPathEngine* pathEngine,
        tSigned32 vertices, const double* vertexCoords,
        std::vector<iShape*>& result
        )
{
    static const tSigned32 NUMBER_OF_ROTATIONS = 30;
    static const double PI = 3.14159265358979323;
    result.resize(NUMBER_OF_ROTATIONS);
    std::vector<tSigned32> rotatedCoords;
    tSigned32 i;
    for(i = 0; i != NUMBER_OF_ROTATIONS; ++i)
    {
        double theta = PI * 2 * i / NUMBER_OF_ROTATIONS;
        RotateShapeCoords(vertices, vertexCoords, theta, rotatedCoords);
        if(!pathEngine->shapeIsValid(&rotatedCoords[0], vertices))
        {
            Error("Fatal", "Shape coordinates are not valid after rotation. Avoid small edges and angles near 180.");
        }
        result[i] = pathEngine->newShape(&rotatedCoords[0], vertices);
    }
}

static iAgent*
RandomlyPlaceObstacle(iMesh* mesh, const cPosition& centre, tSigned32 range, const std::vector<iShape*>& shapeRotations)
{
    int i = rand() % SizeL(shapeRotations);
    cPosition p = mesh->generateRandomPositionLocally(centre, range);
    return mesh->placeAgent(shapeRotations[i], p);
}

static iAgent*
PlaceWithRandomRotation(
        iMesh* mesh, const std::vector<iShape*>& shapeRotations, const cPosition& p
        )
{
    int i = rand() % SizeL(shapeRotations);
    return mesh->placeAgent(shapeRotations[i], p);
}

static void
RandomlyPlaceObstacles(
        iMesh* mesh,
        const cPosition& centre,
        const std::vector<iShape*>& shapeRotations,
        tSigned32 numberToAdd,
        tSigned32 range,
        cDoubleBufferedObstacleSet& addTo
        )
{
    tSigned32 i;
    for(i = 0; i != numberToAdd; ++i)
    {
        iAgent* placed = RandomlyPlaceObstacle(mesh, centre, range, shapeRotations);
        addTo.addObstacle(placed);
    }
}

static void
GenerateWallShapeCoords(
        tSigned32 startX, tSigned32 startY,
        tSigned32 endX, tSigned32 endY,
        tSigned32 radius,
        std::vector<tSigned32>& result
        )
{
    if(startX > endX || (startX == endX && startY > endY))
    {
        std::swap(startX, endX);
        std::swap(startY, endY);
    }
    if(startX == endX || startY == endY)
    {
        result.push_back(startX - radius);
        result.push_back(startY - radius);
        result.push_back(startX - radius);
        result.push_back(endY + radius);
        result.push_back(endX + radius);
        result.push_back(endY + radius);
        result.push_back(endX + radius);
        result.push_back(startY - radius);
        return;
    }
    if(startY > endY)
    {
        result.push_back(startX - radius);
        result.push_back(startY - radius);
        result.push_back(startX - radius);
        result.push_back(startY + radius);
        result.push_back(startX + radius);
        result.push_back(startY + radius);
        result.push_back(endX + radius);
        result.push_back(endY + radius);
        result.push_back(endX + radius);
        result.push_back(endY - radius);
        result.push_back(endX - radius);
        result.push_back(endY - radius);
        return;
    }
    result.push_back(startX + radius);
    result.push_back(startY - radius);
    result.push_back(startX - radius);
    result.push_back(startY - radius);
    result.push_back(startX - radius);
    result.push_back(startY + radius);
    result.push_back(endX - radius);
    result.push_back(endY + radius);
    result.push_back(endX + radius);
    result.push_back(endY + radius);
    result.push_back(endX + radius);
    result.push_back(endY - radius);
}

static iAgent*
PlaceWallAgent(
        const cPosition& start,
        const cPosition& end,
        tSigned32 radius,
        iMesh* mesh
        )
{
    assertD(radius >= 1);
    std::vector<tSigned32> coords;
    GenerateWallShapeCoords(start.x, start.y, end.x, end.y, radius, coords);
    return mesh->placeLargeStaticObstacle(&coords[0], coords.size(), start);
}

static iMesh*
BuildBaseGround(iPathEngine* pathEngine, iTestBed* testBed)
{
  // generate terrain stand-in geometry for the range -10000,-10000 -> 10000,10000
  // world coordinates are in centimetres, so this corresponds to a 200 metres by 200 metres region centred on the origin
    cPartitionedTerrain terrain(-10000, -10000, 2000, 10);
    //cPartitionedTerrain terrain(-30000, -30000, 2000, 30);

    std::vector<const iFaceVertexMesh*> groundParts;
    groundParts.push_back(&terrain);

    const char* options[5];
    options[0] = "partitionTranslationTo3D";
    options[1] = "true";
    options[2] = "useIdentityMapping";
    options[3] = "true";
    options[4] = 0;
    return pathEngine->buildMeshFromContent(&groundParts.front(), SizeL(groundParts), options);
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
            -20,-20,
            -20,20,
            20,20,
            20,-20,
        };
        agentShape = pathEngine->newShape(array, sizeof(array) / sizeof(*array));
    }

    iShape* erasorShape;
    {
        tSigned32 array[]=
        {
            -4000,-4000,
            -4000,4000,
            4000,4000,
            4000,-4000,
        };
        erasorShape = pathEngine->newShape(array, sizeof(array) / sizeof(*array));
    }

    std::vector<iShape*> rockShapes;
    {
        double array[]=
        {
            -180., 60.,
            -60., 100.,
            210., 20.,
            186., -36.,
            120., -48.,
            -180., -60.,
        };
        PreGenerateRotationShapes(pathEngine, sizeof(array) / sizeof(*array), array, rockShapes);
    }
    std::vector<iShape*> hutShapes;
    {
        double array[]=
        {
            -250., -200.,
            -250., 200.,
            250., 200.,
            250., -200.
        };
        PreGenerateRotationShapes(pathEngine, sizeof(array) / sizeof(*array), array, hutShapes);
    }

  // load an existing mesh, if previously saved out
  // or if the file is not present then build a new mesh
    iMesh* mesh;
    if(FileExists("../resource/meshes/semiDynamicObstacles.tok"))
    {
        char* buffer;
        tUnsigned32 size;
        buffer = testBed->loadBinary("../resource/meshes/semiDynamicObstacles.tok", size);
        mesh = pathEngine->loadMeshFromBuffer("tok", buffer, size, 0);
        testBed->freeBuffer(buffer);
    }
    else
    {
        mesh = BuildBaseGround(pathEngine, testBed);
    }

    testBed->setColour("white");
    testBed->printTextLine(10, "Generating base mesh preprocess...");
    testBed->update_ExitIfRequested();

    mesh->generateUnobstructedSpaceFor(erasorShape, false, 0);
    // pre-combining base mesh preprocess can help speed up obstacle set preprocess update
    // in cases where there is base mesh obstruction detail
    // (although this is not actually the case here)
    mesh->generateUnobstructedSpaceFor(agentShape, true, 0);

    testBed->setColour("white");
    testBed->printTextLine(10, "Initialising double buffered obstacle set...");
    testBed->update_ExitIfRequested();

    const char* preprocessAttributes[] =
    {
        "splitWithCircumferenceBelow",
        "5000",
        "smallConvex_WrapNonConvex",
        "true",
        0
    };
    cDoubleBufferedObstacleSet set(testBed, mesh, agentShape, preprocessAttributes);

    testBed->setColour("white");
    testBed->printTextLine(10, "Initialising double buffered obstacle set... (completed)");
    testBed->update_ExitIfRequested();

    testBed->setMesh(mesh); // set this mesh as the current mesh for rendering
    testBed->zoomExtents(); // place camera to see all of newly loaded mesh

    testBed->setColour("white");
    testBed->printTextLine(10, "Initialising dynamic agents...");
    testBed->update_ExitIfRequested();

    set.lockForeground();
#ifdef USER_CONTROLLED_AGENT
    iAgent* userControlledAgent;
    iPath* userControlledAgentPath;
    {
        cPosition p;
        do
        {
            p = mesh->generateRandomPosition();
        }
        while(mesh->testPointCollision(agentShape, set.getForegroundCollisionContext(), p));
        userControlledAgent = mesh->placeAgent(agentShape, p);
        userControlledAgentPath = 0;
    }
#else
    cDynamicAgentManager dynamicAgents(testBed, mesh, agentShape, set.getForegroundCollisionContext(), 1000);
#endif
    set.unlockForeground();

    testBed->setColour("white");
    testBed->printTextLine(10, "Initialising dynamic agents... (completed)");
    testBed->update_ExitIfRequested();

    bool erasorOn = false;
    bool displayTimings = false;

    cAverageTimeTracker moveAlongTimeTracker(40);
    cAverageTimeTracker rePathTimeTracker(40);
    cAverageTimeTracker drawObstaclesTimeTracker(40);
    cAverageTimeTracker drawAgentsTimeTracker(40);
    cAverageTimeTracker assignRegionsTimeTracker(40);
    //double lastUserPathTime = 0.0;

    cPosition wallStart, wallEnd;

    while(1)
    {
    // generate or regenerate set of placed obstacle
        bool regenerate = false;
        do
        {
#ifdef USER_CONTROLLED_AGENT
            if(userControlledAgentPath)
            {
                set.lockForeground();
                iCollisionInfo* ci = userControlledAgent->advanceAlongPath(userControlledAgentPath, 40.f, set.getForegroundCollisionContext());
                if(ci)
                {
                    delete ci;
                    delete userControlledAgentPath;
                    userControlledAgentPath = 0;
                }
                else
                {
                    if(userControlledAgentPath->size() == 1)
                    {
                        delete userControlledAgentPath;
                        userControlledAgentPath = 0;
                    }
                }
                set.unlockForeground();
            }
#else
            {
                set.lockForeground();
                assertR(!set.getForegroundPreprocessedSet()->pathfindPreprocessNeedsUpdate(agentShape));
                clock_t start, finish;
                start = clock();
                dynamicAgents.moveAlongPaths(set.getForegroundCollisionContext());
                finish = clock();
                moveAlongTimeTracker.addTiming(static_cast<double>(finish - start) / CLOCKS_PER_SEC);
                start = clock();
                dynamicAgents.rePath(set.getForegroundPreprocessedSet(), set.getForegroundCollisionContext(), 20);
                finish = clock();
                rePathTimeTracker.addTiming(static_cast<double>(finish - start) / CLOCKS_PER_SEC);
                set.unlockForeground();
            }
            {
                clock_t start, finish;
                start = clock();
                set.lockForeground();
                dynamicAgents.assignRegions(set.getForegroundPreprocessedSet());
                set.unlockForeground();
                finish = clock();
                assignRegionsTimeTracker.addTiming(static_cast<double>(finish - start) / CLOCKS_PER_SEC);
            }
#endif

        // draw mesh
            testBed->setColourRGB(0.0f,0.0f,0.85f);
            testBed->drawMesh();
            testBed->setColour("blue");
            testBed->drawMeshEdges();
            testBed->setColour("white");
            testBed->drawBurntInObstacles();
            testBed->setColour("orange");
            testBed->drawMeshExpansion(agentShape);

        // draw obstacles in double buffered set
            {
                clock_t start, finish;
                set.lockForeground();
                start = clock();
                testBed->setColour("white");
                set.renderPreprocessed();
                testBed->setColour("orange");
                set.renderDynamic();
                testBed->setColour("red");
                set.renderToBeDeleted();
                testBed->setColour("orange");
                set.renderPreprocessedExpansion();
                finish = clock();
                drawObstaclesTimeTracker.addTiming(static_cast<double>(finish - start) / CLOCKS_PER_SEC);
                set.unlockForeground();
            }

#ifdef USER_CONTROLLED_AGENT
            testBed->setColour("green");
            testBed->drawAgent(userControlledAgent, 60);
            testBed->drawPath(userControlledAgentPath);
#else
            {
                clock_t start, finish;
                start = clock();
                dynamicAgents.renderAgents_ColouredByRegion();
                finish = clock();
                drawAgentsTimeTracker.addTiming(static_cast<double>(finish - start) / CLOCKS_PER_SEC);
            }
#endif

            testBed->setColour("orange");
            {
                std::ostringstream oss;
                set.lockForeground();
                oss << set.numberOfPreprocessedObstacles() << " preprocessed obstacles, ";
                oss << set.numberOfDynamicObstacles() << " dynamic obstacles, ";
                oss << set.getForegroundPreprocessedSet()->getNumberOfConnectedRegions(agentShape) << " connected regions";
#ifndef USER_CONTROLLED_AGENT
                oss << ", " << dynamicAgents.size()  << " moving agents";
#endif
                set.unlockForeground();
                testBed->printTextLine(0, oss.str().c_str());
            }
            if(set.updateInProgress())
            {
                testBed->printTextLine(0, "Preprocess update in progress..");
            }
            if(displayTimings)
            {
                testBed->setColour("purple");
                if(!set.updateInProgress())
                {
                    std::ostringstream oss;
                    oss << "last preprocess update time: " << set.getLastUpdateTime();
                    testBed->printTextLine(0, oss.str().c_str());
                    //{
                    //    std::ostringstream oss;
                    //    oss << "last buffer swap time: " << set.getLastBufferSwapTime();
                    //    testBed->printTextLine(10, oss.str().c_str());
                    //}
                }
                if(moveAlongTimeTracker.ready())
                {
                    std::ostringstream oss;
                    oss << "advance along paths (average per frame): " << moveAlongTimeTracker.get() << "s";
                    testBed->printTextLine(0, oss.str().c_str());
                }
                if(rePathTimeTracker.ready())
                {
                    std::ostringstream oss;
                    oss << "new path generation (average per frame): " << rePathTimeTracker.get() << "s";
                    testBed->printTextLine(0, oss.str().c_str());
                }
                if(drawObstaclesTimeTracker.ready())
                {
                    std::ostringstream oss;
                    oss << "draw obstacles (average per frame): " << drawObstaclesTimeTracker.get() << "s";
                    testBed->printTextLine(0, oss.str().c_str());
                }
                if(drawAgentsTimeTracker.ready())
                {
                    std::ostringstream oss;
                    oss << "draw agents (average per frame): " << drawAgentsTimeTracker.get() << "s";
                    testBed->printTextLine(0, oss.str().c_str());
                }
                if(assignRegionsTimeTracker.ready())
                {
                    std::ostringstream oss;
                    oss << "assign regions (average per frame): " << assignRegionsTimeTracker.get() << "s";
                    testBed->printTextLine(0, oss.str().c_str());
                }
#ifdef USER_CONTROLLED_AGENT
                {
                    std::ostringstream oss;
                    oss << "last user selected path time: " << lastUserPathTime << "s";
                    testBed->printTextLine(0, oss.str().c_str());
                }
#endif
            }
            testBed->setColour("lightgrey");
            testBed->printTextLine(0, "Press 'T' to toggle timing info");

        // render anything that needs to be displayed with additive blending

            testBed->enterAdditiveBlendingPhase();

            if(wallStart.cell != -1)
            {
                testBed->setColour("green");
                iAgent* a;
                if(testBed->getKeyState("_LSHIFT"))
                {
                    a = PlaceWallAgent(wallStart, wallEnd, 150, mesh);
                }
                else
                {
                    a = PlaceWallAgent(wallStart, wallEnd, 100, mesh);
                }
                testBed->setAdditiveBlendingAlpha(0.5f);
                testBed->drawAgent(a, 20);
                delete a;
            }

            if(erasorOn)
            {
                cPosition p = testBed->positionAtMouse();
                if(p.cell != -1)
                {
                    testBed->setColour("red");
                    testBed->setAdditiveBlendingAlpha(0.5f);
                    testBed->drawShape(erasorShape, p, 20);
                }
            }

        // tell the testBed to render this frame
            testBed->update_ExitIfRequested();

        // receive and process messages for all keys pressed since last frame
            cPosition p = testBed->positionAtMouse();
            const char* keyPressed;
            while(keyPressed = testBed->receiveKeyMessage())
            {
                if(keyPressed[0] != 'd') // is it a key down message?
                    continue;

                switch(keyPressed[1])
                {
                case '_':
                    {
                        if(!strcmp("ESCAPE", keyPressed + 2))
                        {
                            return;
                        }
                        if(!strcmp("SPACE", keyPressed + 2) && !set.updateInProgress())
                        {
                            regenerate = true;
                        }
                        break;
                    }
                case 'H':
                    if(p.cell != -1)
                    {
                        if(testBed->getKeyState("_LSHIFT"))
                        {
                            RandomlyPlaceObstacles(mesh, p, hutShapes, 30, 5000, set);
                        }
                        else
                        {
                            set.addObstacle(PlaceWithRandomRotation(mesh, hutShapes, p));
                        }
                    }
                    break;
                case 'R':
                    if(p.cell != -1)
                    {
                        if(testBed->getKeyState("_LSHIFT"))
                        {
                            RandomlyPlaceObstacles(mesh, p, rockShapes, 25, 3000, set);
                        }
                        else
                        {
                            set.addObstacle(PlaceWithRandomRotation(mesh, rockShapes, p));
                        }
                    }
                    break;
                case 'W':
                    if(wallStart.cell != -1)
                    {
                        if(testBed->getKeyState("_LSHIFT"))
                        {
                            set.addObstacle(PlaceWallAgent(wallStart, wallEnd, 150, mesh));
                        }
                        else
                        {
                            set.addObstacle(PlaceWallAgent(wallStart, wallEnd, 100, mesh));
                        }
                        wallStart = wallEnd;
                    }
                    break;
                case 'D':
                    if(p.cell != -1)
                    {
                        set.lockForeground();
                        cVectorReceiver<iAgent*> agentsReceiver;
                        mesh->getAllAgentsOverlapped(
                                erasorOn ? erasorShape : agentShape,
                                set.getForegroundCollisionContext(),
                                p,
                                agentsReceiver
                                );
                        set.unlockForeground();
                        for(tSigned32 i = 0; i != agentsReceiver.vector.size(); ++i)
                        {
                            set.removeAndDeleteObstacle(agentsReceiver.vector[i]);
                        }
                    }
                    break;
                case 'E':
                    erasorOn = !erasorOn;
                    break;
                case 'T':
                    displayTimings = !displayTimings;
                    break;
                case 'C':
                    if(p.cell != -1)
                    {
                        if(testBed->getKeyState("_LSHIFT") || wallStart.cell == -1)
                        {
                          // start new wall
                            wallStart = p;
                        }
                        wallEnd = p;
                    }
                    break;
                case 'S':
                    {
                        set.storeToNamedObstacles();
                        cFileOutputStream fos("../resource/meshes/semiDynamicObstacles.tok");
                        mesh->saveGround("tok", true, fos);
                    }
                    break;
#ifdef USER_CONTROLLED_AGENT
                case 'P':
                    delete userControlledAgentPath;
                    userControlledAgentPath = 0;
                    if(p.cell != -1)
                    {
                        set.lockForeground();
                        cPosition target = mesh->findClosestUnobstructedPosition(
                                agentShape,
                                set.getForegroundCollisionContext(),
                                p, 100
                                );
                        if(target.cell != -1)
                        {
                            const tSigned32 numberOfIterations = 100;
                            tSigned32 i;
                            clock_t start, finish;
                            start = clock();
                            for(i = 0; i != numberOfIterations; ++i)
                            {
                                delete userControlledAgentPath;
                                userControlledAgentPath = userControlledAgent->findShortestPathTo(
                                        set.getForegroundCollisionContext(),
                                        target
                                        );
                            }
                            finish = clock();
                            lastUserPathTime = static_cast<double>(finish - start) / CLOCKS_PER_SEC / numberOfIterations;
                        }
                        set.unlockForeground();
                    }
                    break;
#endif
                }
            }
        }
        while(regenerate == false);
        set.startUpdate();
    }
}
