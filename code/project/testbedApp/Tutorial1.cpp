
#include "base/Header.h"
#include "sampleShared/Error.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "externalAPI/i_pathengine.h"
#include "externalAPI/i_testbed.h"
#include <string.h>

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

    iMesh* mesh;
    {
        char *buffer;
        tUnsigned32 size;
        buffer=testbed->loadBinary("../resource/meshes/mesh1.xml",size);
        mesh=pathengine->loadMeshFromBuffer("xml", buffer, size, 0);
        testbed->freeBuffer(buffer);
    }
    testbed->setMesh(mesh);
    testbed->zoomExtents();

// create shapes
    iShape *agent_shape;
    iShape *obstruction_shape1;
    iShape *obstruction_shape2;
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
        agent_shape=pathengine->newShape(array, sizeof(array) / sizeof(*array));
    }
    {
        tSigned32 array[]=
        {
            30,30,
            30,-30,
            -30,-30,
            -30,30,
        };
        obstruction_shape1=pathengine->newShape(array, sizeof(array) / sizeof(*array));
    }
    {
        tSigned32 array[]=
        {
            10,35,
            20,-20,
            -10,-30,
            -15,32,
        };
        obstruction_shape2=pathengine->newShape(array, sizeof(array) / sizeof(*array));
    }

// generate collision preprocess for the agent shape
    mesh->generateUnobstructedSpaceFor(agent_shape, false, 0);

// iAgent objects are used to instantiate shapes on a mesh - either as true agents or as obstructions
// values are initialised to 0 here to indicate that nothing has yet been placed
    iAgent* agent = 0;
    iAgent* obstruction_agent1 = 0;
    iAgent *obstruction_agent2 = 0;

// iCollisionContext objects encapsulate a state of collision on a mesh - essentially this is a set of agents
    iCollisionContext* context = mesh->newContext();

    bool exitFlag = false;
    while(!exitFlag)
    {
        testbed->setColourRGB(0.0f,0.0f,0.85f);
        testbed->drawMesh();
        testbed->setColour("blue");
        testbed->drawMeshEdges();
        testbed->setColour("white");
        testbed->drawBurntInObstacles();

    // draw agent if it has been placed
        if(agent)
        {
            testbed->setColour("white");
            testbed->drawAgent(agent,20);
        }

    // draw any obstructions that have been placed
        if(obstruction_agent1)
        {
            testbed->setColour("red");
            testbed->drawAgent(obstruction_agent1,20);
        }
        if(obstruction_agent2)
        {
            testbed->setColour("red");
            testbed->drawAgent(obstruction_agent2,20);
        }

        testbed->update(exitFlag);

    // receive and process messages for all keys pressed since last frame
        const char *keypressed;
        while(keypressed=testbed->receiveKeyMessage())
        {
            if(keypressed[0]!='d') // is it a key down message?
                continue;

            switch(keypressed[1])
            {
            case '_':
                {
                    if(strcmp("ESCAPE",keypressed+2)==0)
                        exitFlag=true;
                    break;
                }
            case '4':
            // place or move the agent
                {
                    cPosition p=testbed->positionAtMouse();
                    if(p.cell!=-1 && !mesh->testPointCollision(agent_shape,context,p))
                    {
                        if(!agent)
                            agent=mesh->placeAgent(agent_shape,p);
                        else
                            agent->moveTo(p);
                    }
                }
                break;
            case '5':
            // place or move obstruction 1
                {
                    cPosition p=testbed->positionAtMouse();
                    if(p.cell!=-1 && (agent==0 || !agent->testCollisionDirectlyAgainstPlacedShape(obstruction_shape1, p)))
                    {
                    // place the agent if not yet placed, otherwise move to the requested position
                        if(!obstruction_agent1)
                        {
                            obstruction_agent1=mesh->placeAgent(obstruction_shape1,p);
                            context->addAgent(obstruction_agent1);
                        }
                        else
                            obstruction_agent1->moveTo(p);
                    }
                }
                break;
            case '6':
            // place or move obstruction 2
                {
                    cPosition p=testbed->positionAtMouse();
                    if(p.cell!=-1 && (agent==0 || !agent->testCollisionDirectlyAgainstPlacedShape(obstruction_shape2, p)))
                    {
                    // place the agent if not yet placed, otherwise move to the requested position
                        if(!obstruction_agent2)
                        {
                            obstruction_agent2=mesh->placeAgent(obstruction_shape2,p);
                            context->addAgent(obstruction_agent2);
                        }
                        else
                            obstruction_agent2->moveTo(p);
                    }
                }
                break;
            }
        }
    }
}
