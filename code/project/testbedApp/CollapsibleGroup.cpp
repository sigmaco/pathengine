#include "base/Header.h"
#include "project/testbedApp/CollapsibleGroup/GetGroupRootPosition.h"
#include "project/testbedApp/CollapsibleGroup/GenerateColumnTargets.h"
#include "project/testbedApp/CollapsibleGroup/ColumnTargetsManager.h"
#include "project/testbedApp/CollapsibleGroup/FormationMovementOrder.h"
#include "project/testbedApp/CollapsibleGroup/FormationMovementParameters.h"
#include "sampleShared/WaitForFrame.h"
#include "sampleShared/SelectAndMove.h"
#include "sampleShared/Error.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "externalAPI/i_testbed.h"
#include "externalAPI/i_pathengine.h"
#include <vector>
#include <string.h>

void
TestbedApplicationMain(iPathEngine* pathEngine, iTestBed* testbed)
{
// check if interfaces are compatible with the headers used for compilation
    if(testbed->getInterfaceMajorVersion()!=TESTBED_INTERFACE_MAJOR_VERSION
        ||
        testbed->getInterfaceMinorVersion()<TESTBED_INTERFACE_MINOR_VERSION)
    {
        testbed->reportError("Fatal","Testbed version is incompatible with headers used for compilation.");
        return;
    }
    if(pathEngine->getInterfaceMajorVersion()!=PATHENGINE_INTERFACE_MAJOR_VERSION
        ||
        pathEngine->getInterfaceMinorVersion()<PATHENGINE_INTERFACE_MINOR_VERSION)
    {
        testbed->reportError("Fatal","Pathengine version is incompatible with headers used for compilation.");
        return;
    }

  // configuration parameters
    const tSigned32 numberOfAgents = 80;
    const tSigned32 positionResolveDistance = 1500; // distance to use when resolving clicked positions
    const tSigned32 agentRadius = 10;
    const tSigned32 agentStartPlacementDistance = 200; // range to use when placing agents initially

  // parameters for the actual group movement code supplied through the following class
  // (default values are set by the class constructor)
    cFormationMovementParameters parameters;

  // create shapes
    iShape* agentShape;
    {
        tSigned32 array[]=
        {
            -agentRadius,agentRadius,
            agentRadius,agentRadius,
            agentRadius,-agentRadius,
            -agentRadius,-agentRadius,
        };
        agentShape = pathEngine->newShape(array, sizeof(array) / sizeof(*array));
    }

    iMesh* mesh;
    {
        char *buffer;
        tUnsigned32 size;
        buffer = testbed->loadBinary("../resource/meshes/thainesford_town.tok", size);
        mesh = pathEngine->loadMeshFromBuffer("tok", buffer, size, 0);
        testbed->freeBuffer(buffer);
    }

    testbed->setMesh(mesh); // set this mesh as the current mesh for rendering
    testbed->zoomExtents(); // pl1ace camera to see all of newly loaded mesh

  // generate collision and pathfinding preprocess for the agent shape
    mesh->generateUnobstructedSpaceFor(agentShape, true, 0);
    mesh->generatePathfindPreprocessFor(agentShape, 0);

  // iCollisionContext objects encapsulate a state of collision on a mesh - essentially this is a collection of agents
  // this context will be used for collision and will contain the agent and all swarmers
    iCollisionContext* context = mesh->newContext();

  // iAgent objects are used to instantiate shapes on a mesh - either as true agents or as obstructions
    std::vector<iAgent*> agents;
    tSigned32 i;
    {
        cPosition placementCentre;
        do
        {
            placementCentre = mesh->generateRandomPosition();
        }
        while(mesh->testPointCollision(agentShape, context, placementCentre));
        for(i = 0; i != numberOfAgents; ++i)
        {
            cPosition p;
            do
            {
                p = mesh->generateRandomPositionLocally(placementCentre, agentStartPlacementDistance);
            }
            while(mesh->testPointCollision(agentShape, context, p));
            agents.push_back(mesh->placeAgent(agentShape, p));
        }
    }

    std::vector<tSigned32> selectedAgents;
    std::vector<bool> agentSelected(numberOfAgents, false);

    cSelectAndMove selectAndMove;

  // paths
    iPath* groupPath = 0;

    cFormationMovementOrder* movementOrder = 0;
    std::vector<tSigned32> agentIndexAssignments;

    cPosition visualisePathStart, visualisePathEnd;
    visualisePathStart.cell = -1;
    visualisePathEnd.cell = -1;
    iPath* visualisePath = 0;
    iCollisionContext* visualiseContext = mesh->newContext();

    bool paused = false;
    bool doSingleStep = false;
    bool exitFlag = false;
    while(!exitFlag)
    {
    // generate visualisation path and agents if updated
        if(visualisePath == 0 && visualisePathStart.cell != -1 && visualisePathEnd.cell != -1)
        {
            // delete any previous visualisation agents
            while(visualiseContext->getNumberOfAgents())
            {
                delete visualiseContext->getAgent(0);
            }
            assertD(!mesh->testPointCollision(agentShape, 0, visualisePathStart));
            visualisePath = mesh->findShortestPath(agentShape, context, visualisePathStart, visualisePathEnd);
            if(visualisePath && visualisePath->size() > 1)
            {
                const cPosition* positionArray = visualisePath->getPositionArray();
                const tSigned32* connectionIndexArray = visualisePath->getConnectionIndexArray();
                iPath* copiedPath = mesh->constructPath(positionArray,  visualisePath->size(), connectionIndexArray, connectionIndexArray ? visualisePath->size() - 1 : 0);

                cColumnTargetsManager ctm(
                        mesh, agentShape, context, copiedPath,
                        parameters._agentSpacing, parameters._maximumWidth,
                        parameters._groupStepDistance
                        );
                do
                {
                    tSigned32 added = ctm.pushFront();
                    cRowTargets& row = ctm.refRow(added);
                    for(i = row.firstColumn(); i <= row.lastColumn(); ++i)
                    {
                        visualiseContext->addAgent(mesh->placeAgent(agentShape, row.position(i)));
                    }
                    ctm.popBack(added);
                }
                while(!ctm.atEnd());
            }
        }

    // draw mesh
        testbed->setColourRGB(0.0f,0.0f,0.85f);
        testbed->drawMesh();
        testbed->setColour("blue");
        testbed->drawMeshEdges();
        testbed->setColour("white");
        testbed->drawBurntInObstacles();
        testbed->setColour("brown");
        testbed->drawMeshExpansion(agentShape);

    // draw agents
        for(i = 0; i != numberOfAgents; ++i)
        {
            if(agentSelected[i])
            {
                testbed->setColour("orange");
            }
            else
            {
                testbed->setColour("grey");
            }
            testbed->drawAgent(agents[i], 20);
        }

    // movement order debug draw
        if(movementOrder)
        {
            testbed->setColour("lightblue");
            movementOrder->debugDraw(testbed);
        }

    // draw group path (if set)
        testbed->setColour("green");
        testbed->drawPath(groupPath);

    // draw visualisation path and agents, if any
        testbed->setColour("green");
        testbed->drawPosition(visualisePathStart, 30);
        testbed->drawPosition(visualisePathEnd, 30);
        if(visualisePath)
        {
            testbed->drawPath(visualisePath);
            testbed->setColour("green");
            testbed->drawAgentsInContext(visualiseContext, 20);
        }

    // update / draw click select
        {
            bool completed;
            selectAndMove.update(*testbed, completed);
            if(completed)
            {
                std::vector<tSigned32> screenPositions(numberOfAgents * 2);
                for(i = 0; i != numberOfAgents; ++i)
                {
                    cPosition p = agents[i]->getPosition();
                    tSigned32 worldPosition[3];
                    worldPosition[0] = p.x;
                    worldPosition[1] = p.y;
                    worldPosition[2] = mesh->heightAtPosition(p);
                    testbed->worldToScreen(worldPosition, screenPositions[i * 2], screenPositions[i * 2 + 1]);
                }
                selectAndMove.updateSelectionSet(*testbed, screenPositions, agentSelected, selectedAgents);
            }
        }


    // update movement order
        if(movementOrder && (!paused || doSingleStep))
        {
            bool completed = movementOrder->advance();
            for(tSigned32 i = 0; i != numberOfAgents; ++i)
            {
                tSigned32 indexAssignment = agentIndexAssignments[i];
                if(indexAssignment != -1)
                {
                    agents[i]->moveTo(movementOrder->getCurrentPosition(indexAssignment));
                }
            }
            if(completed)
            {
                delete movementOrder;
                movementOrder = 0;
            }
        }
        doSingleStep = false;

    // tell the testbed to render this frame
        testbed->update(exitFlag);

    // frame rate cap
        WaitForFrame();

    // receive and process messages for all keys pressed since last frame
        const char* keyPressed;
        while(keyPressed = testbed->receiveKeyMessage())
        {
            {
                bool handled, moveRequested;
                selectAndMove.handleInputMessage(*testbed, keyPressed, handled, moveRequested);
                if(handled)
                {
                    if(moveRequested)
                    {
                        delete movementOrder;
                        movementOrder = 0;

                        if(selectedAgents.empty())
                        {
                            break;
                        }

                        cPosition targetPosition = testbed->positionAtMouse();

                        if(targetPosition.cell == -1)
                        {
                            break;
                        }
                        targetPosition = mesh->findClosestUnobstructedPosition(agentShape, context, targetPosition, positionResolveDistance);
                        if(targetPosition.cell == -1)
                        {
                            break;
                        }

                        std::vector<cPosition> positions(selectedAgents.size());
                        for(tSigned32 i = 0; i != SizeL(selectedAgents); ++i)
                        {
                            tSigned32 j = selectedAgents[i];
                            if(agents[j]->testCollisionAt(0, agents[j]->getPosition()))
                            {
                                cPosition p = agents[j]->findClosestUnobstructedPosition(0, positionResolveDistance);
                                if(p.cell != -1)
                                {
                                    agents[j]->moveTo(p);
                                }
                            }
                            positions[i] = agents[j]->getPosition();
                        }
                        std::vector<tSigned32> indexAssignments;
                        movementOrder = new cFormationMovementOrder(
                                mesh, agentShape, context,
                                parameters,
                                positions,
                                targetPosition,
                                indexAssignments
                                );
                        agentIndexAssignments.resize(0);
                        agentIndexAssignments.resize(agents.size(), -1);
                        for(tSigned32 i = 0; i != SizeL(selectedAgents); ++i)
                        {
                            agentIndexAssignments[selectedAgents[i]] = indexAssignments[i];
                        }
                    }
                    continue;
                }
            }

            if(keyPressed[0] != 'd') // is it a key down message?
                continue;

            switch(keyPressed[1])
            {
            case 'S':
                visualisePathStart = testbed->positionAtMouse();
                delete visualisePath;
                visualisePath = 0;
                break;
            case 'E':
                visualisePathEnd = testbed->positionAtMouse();
                delete visualisePath;
                visualisePath = 0;
                break;
            case 'P':
                paused = !paused;
                break;
            case 'A':
                doSingleStep = true;
                break;
            case '_':
                if(!strcmp("ESCAPE", keyPressed + 2))
                {
                    exitFlag = true;
                    break;
                }
            }
        }
    }

    delete movementOrder;
}
