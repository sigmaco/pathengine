//**********************************************************************
//
// Copyright (c) 2012
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include "base/Header.h"
#include "project/testbedApp/SeparatingGroups/SeparatingGroups.h"
#include "project/testbedApp/SeparatingGroups/AgentAI.h"
#include "sampleShared/SelectAndMove.h"
#include "sampleShared/WaitForFrame.h"
#include "base/Geometry/HorizontalRange_Helper.h"
#include "externalAPI/i_testbed.h"
#include "externalAPI/i_pathengine.h"
#include <sstream>
#include <vector>
#include <string.h>

static iShape*
SquareShape(iPathEngine& pathEngine, tSigned32 radius)
{
    tSigned32 array[]=
    {
        -radius,radius,
        radius,radius,
        radius,-radius,
        -radius,-radius,
    };
    return pathEngine.newShape(array, sizeof(array) / sizeof(*array));
}

void
SeparatingGroups(iTestBed& testbed, iPathEngine& pathEngine)
{
    const tSigned32 clickResolveDistance = 1500;

    iShape* agentShape = SquareShape(pathEngine, 10);
    iShape* avoidShape = SquareShape(pathEngine, 20);

    iMesh* mesh;
    {
        char *buffer;
        tUnsigned32 size;
        buffer = testbed.loadBinary("../resource/meshes/mesh1.tok", size);
        mesh = pathEngine.loadMeshFromBuffer("tok", buffer, size, 0);
        testbed.freeBuffer(buffer);
    }

    testbed.setMesh(mesh); // set this mesh as the current mesh for rendering
    testbed.zoomExtents(); // pl1ace camera to see all of newly loaded mesh

    mesh->generateUnobstructedSpaceFor(agentShape, true, 0);
    mesh->generatePathfindPreprocessFor(agentShape, 0);

    cAgentAI ai(mesh, agentShape, avoidShape);

    std::vector<tSigned32> selectedAgents;
    std::vector<bool> agentSelected(ai.numberOfAgents(), false);

    std::vector<cPosition> waypointsForOrderInProgress;

    cSelectAndMove selectAndMove;

    bool paused = false;
    bool doSingleStep = false;
    bool exitFlag = false;
    while(!exitFlag)
    {
    // draw mesh
        testbed.setColourRGB(0.0f,0.0f,0.85f);
        testbed.drawMesh();
        testbed.setColour("blue");
        testbed.drawMeshEdges();
        testbed.setColour("white");
        testbed.drawBurntInObstacles();
        testbed.setColour("brown");
        testbed.drawMeshExpansion(agentShape);

    // draw agents
        tSigned32 i;
        for(i = 0; i != ai.numberOfAgents(); ++i)
        {
            {
                tSigned32 groupIndex = ai.agentGroupIndex(i);
                if(groupIndex == -1)
                {
                    testbed.setColour("orange");
                }
                else
                {
                    switch(groupIndex % 4)
                    {
                    default:
                        invalid();
                    case 0:
                        testbed.setColour("red");
                        break;
                    case 1:
                        testbed.setColour("purple");
                        break;
                    case 2:
                        testbed.setColour("brown");
                        break;
                    case 3:
                        testbed.setColour("pink");
                        break;
                    }
                }
            }
            testbed.drawAgent(ai.getAgent(i), 20);
            if(agentSelected[i])
            {
                testbed.setColour("lightblue");
                testbed.drawAgent(ai.getHighlightAgent(i), 3);
            }
        }

        {
            bool completed;
            selectAndMove.update(testbed, completed);
            if(completed)
            {
                std::vector<tSigned32> screenPositions(ai.numberOfAgents() * 2);
                for(i = 0; i != ai.numberOfAgents(); ++i)
                {
                    cPosition p = ai.getAgent(i)->getPosition();
                    tSigned32 worldPosition[3];
                    worldPosition[0] = p.x;
                    worldPosition[1] = p.y;
                    worldPosition[2] = mesh->heightAtPosition(p);
                    testbed.worldToScreen(worldPosition, screenPositions[i * 2], screenPositions[i * 2 + 1]);
                }
                selectAndMove.updateSelectionSet(testbed, screenPositions, agentSelected, selectedAgents);
                waypointsForOrderInProgress.clear();
            }
        }

        testbed.setColour("red");
        for(tSigned32 i = 0; i != SizeL(waypointsForOrderInProgress); ++i)
        {
            testbed.drawPosition(waypointsForOrderInProgress[i], 20);
        }

        ai.update();

        doSingleStep = false;

    // tell the testbed to render this frame
        testbed.update(exitFlag);

    // frame rate cap
        WaitForFrame();

    // receive and process messages for all keys pressed since last frame
        const char* keyPressed;
        while(keyPressed = testbed.receiveKeyMessage())
        {
            {
                bool handled, moveRequested;
                selectAndMove.handleInputMessage(testbed, keyPressed, handled, moveRequested);
                if(handled)
                {
                    if(moveRequested)
                    {
                        cPosition targetPosition = testbed.positionAtMouse();
                        if(targetPosition.cell != -1)
                        {
                            targetPosition = mesh->findClosestUnobstructedPosition(agentShape, 0, targetPosition, clickResolveDistance);
                        }
                        if(testbed.getKeyState("_SHIFT"))
                        {
                            if(targetPosition.cell != -1)
                            {
                                waypointsForOrderInProgress.push_back(targetPosition);
                            }
                            continue;
                        }
                        assertD(waypointsForOrderInProgress.empty());
                        if(targetPosition.cell == -1)
                        {
                            ai.stopOrder(selectedAgents);
                            continue;
                        }
                        ai.moveOrder(selectedAgents, targetPosition);
                    }
                    continue;
                }
            }

            if(keyPressed[0] != 'd') // is it a key down message?
            {
                if(!strcmp("_SHIFT", keyPressed + 1) && !selectAndMove.isActive() && !waypointsForOrderInProgress.empty())
                {
                    ai.moveOrder(selectedAgents, waypointsForOrderInProgress, waypointsForOrderInProgress.size() > 1);
                    waypointsForOrderInProgress.clear();
                }
                continue;
            }

            switch(keyPressed[1])
            {
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
}
