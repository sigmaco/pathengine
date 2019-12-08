
#include "base/Header.h"
#include "sampleShared/Error.h"
#include "sampleShared/MoveAgent.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "externalAPI/i_pathengine.h"
#include "externalAPI/i_testbed.h"
#include <vector>
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

    bool exitFlag=false;
    
// create shapes
    iShape *agent_shape;
    iShape *swarmer_shape;
    iShape *danger_shape;
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
            0,16,
            14,-8,
            -14,-8,
        };
        swarmer_shape=pathengine->newShape(array, sizeof(array) / sizeof(*array));
    }
    {
        tSigned32 array[]=
        {
            -60,60,
            60,60,
            60,-60,
            -60,-60,
        };
        danger_shape=pathengine->newShape(array, sizeof(array) / sizeof(*array));
    }

    iMesh* mesh;
    {
        char *buffer;
        tUnsigned32 size;
        buffer=testbed->loadBinary("../resource/meshes/mesh1.tok", size);
        mesh=pathengine->loadMeshFromBuffer("tok", buffer, size, 0);
        testbed->freeBuffer(buffer);
    }

    testbed->setMesh(mesh); // set this mesh as the current mesh for rendering
    testbed->zoomExtents(); // pl1ace camera to see all of newly loaded mesh

// generate just collision preprocess for the agent shape, and collision and pathfinding preprocess for the swarmers
    mesh->generateUnobstructedSpaceFor(agent_shape, false, 0);
    mesh->generateUnobstructedSpaceFor(swarmer_shape, true, 0);
    mesh->generatePathfindPreprocessFor(swarmer_shape, 0);

// iAgent objects are used to instantiate shapes on a mesh - either as true agents or as obstructions
// we initialise values to 0 here to indicate that nothing has yet been placed
    iAgent* agent = 0;
    std::vector<iAgent*> swarmer_agents;

// paths for swarmers
    std::vector<iPath*> swarmer_paths;

// iCollisionContext objects encapsulate a state of collision on a mesh - essentially this is a collection of agents
// this context will be used for collision and will contain the agent and all swarmers
    iCollisionContext* context = mesh->newContext();

// direction agent is currently facing
    float agent_heading = 0;

// swarmers take turns to pathfind
// this variable holds the index of the next swarmer to pathfind
    tSigned32 next_swarmer = 0;

// this context will be used for queries around a blocking agent
    iCollisionContext* temporary_context = mesh->newContext();

    bool swarmers_attack = true;

    while(!exitFlag)
    {
    // draw mesh
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
            testbed->setColour("orange");
            testbed->drawAgentHeading(agent, 30, 20, agent_heading);
        }

    // draw any swarmers which have been placed
        testbed->setColour("green");
        for(tSigned32 i = 0; i < SizeL(swarmer_agents); i++)
        {
            testbed->drawAgent(swarmer_agents[i], 20);
        }

    // tell the testbed to render this frame
        testbed->update(exitFlag);

    // move the agent under the control of user input
        if(agent)
        {
            TurnAgentUnderMouseControl(testbed, agent_heading);
            MoveAgentUnderKeyControl(testbed, pathengine, agent, agent_heading, context, 6.0f);
        }

    // move swarmers
        if(agent)
        {
            tSigned32 i;
            for(i = 0; i < static_cast<tSigned32>(swarmer_agents.size()); i++)
            {
                if(i == next_swarmer)
                {
                // this swarmer is allowed to pathfind this frame

                    delete swarmer_paths[i];
                    if(swarmers_attack)
                    {
                        swarmer_paths[i] = swarmer_agents[i]->findShortestPathTo(0, agent->getPosition());
                    }
                    else
                    {
                        iAgent* danger = mesh->placeAgent(danger_shape, agent->getPosition());
                        temporary_context->addAgent(danger);
                        swarmer_paths[i] = swarmer_agents[i]->findPathAway(temporary_context, agent->getPosition(), 400);
                        delete danger;
                    }

                    iCollisionInfo* collisionInfo = swarmer_agents[i]->advanceAlongPath(swarmer_paths[i], 5.f, context);

                    if(collisionInfo)
                    {
                        iAgent* blocking_agent = collisionInfo->getCollidingAgent();
                        delete collisionInfo;

                        if(swarmers_attack && blocking_agent == agent)
                        {
                            blocking_agent = 0;
                        }

                        if(blocking_agent)
                        {
                            temporary_context->addAgent(blocking_agent);

                            tUnsigned32 length_before = swarmer_paths[i]->getLength();
                            delete swarmer_paths[i];

                            if(swarmers_attack)
                            {
                                swarmer_paths[i] = swarmer_agents[i]->findShortestPathTo(temporary_context, agent->getPosition());
                            }
                            else
                            {
                                iAgent* danger = mesh->placeAgent(danger_shape, agent->getPosition());
                                temporary_context->addAgent(danger);
                                swarmer_paths[i] = swarmer_agents[i]->findPathAway(temporary_context, agent->getPosition(), 400);
                                delete danger;
                            }

                            tUnsigned32 length_after = 0;
                            if(swarmer_paths[i])
                            {
                                length_after = swarmer_paths[i]->getLength();
                            }

                            if(length_after < length_before + 40)
                            {
                                collisionInfo = swarmer_agents[i]->advanceAlongPath(swarmer_paths[i],5.0f,context);
                                delete collisionInfo;
                            }
                            else
                            {
                                delete swarmer_paths[i];
                                swarmer_paths[i]=0;
                            }

                            temporary_context->removeAgent(blocking_agent);
                        }
                    }


                }
                else
                {
                // this swarmer only advances along their current path if possible
                    iCollisionInfo* collisionInfo = swarmer_agents[i]->advanceAlongPath(swarmer_paths[i], 5.0f, context);
                    delete collisionInfo;
                }
            }
            next_swarmer++;
            if(next_swarmer >= static_cast<tSigned32>(swarmer_agents.size()))
            {
                next_swarmer = 0;
            }
        }


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
                    if(!strcmp("ESCAPE",keypressed+2))
                    {
                        exitFlag=true;
                        break;
                    }
                    if(!strcmp("SPACE",keypressed+2))
                    {
                        swarmers_attack=!swarmers_attack;
                        break;
                    }
                    break;
                }
            case '4':
            // place or move the player controlled agent
                {
                // this is cached by the testbed so it is ok to call multiple times
                    cPosition p=testbed->positionAtMouse();
                    if(p.cell!=-1)
                    {
                    // place the agent if not yet placed, otherwise move to the requested position
                        if(!agent)
                        {
                            if(!mesh->testPointCollision(agent_shape,context,p))
                            {
                                agent=mesh->placeAgent(agent_shape,p);
                                context->addAgent(agent);
                            }
                        }
                        else
                        {
                            if(!agent->testCollisionAt(context,p))
                                agent->moveTo(p);
                        }
                    }
                }
                break;
            case '5':
            // place a new swarmer
                if(swarmer_agents.size()<30)
                {
                // get the position on the mesh under the mouse
                // p.cell is set to -1 if the mouse is not over the mesh
                    cPosition p=testbed->positionAtMouse();
                    if(p.cell!=-1 && !mesh->testPointCollision(swarmer_shape,context,p))
                    {
                        swarmer_agents.push_back(mesh->placeAgent(swarmer_shape,p));
                        context->addAgent(swarmer_agents.back());
                        swarmer_paths.push_back(0);
                    }
                }
                break;
            }
        }
    }
}
