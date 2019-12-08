#include "base/Header.h"
#include "project/testbedApp/PlayableDemo/PackHunterBehaviour.h"
#include "project/testbedApp/PlayableDemo/Globals.h"
#include "project/testbedApp/PlayableDemo/GameState.h"
#include "project/testbedApp/PlayableDemo/GameObject.h"
#include "project/testbedApp/PlayableDemo/PathFrequency.h"
#include "sampleShared/MoveAgent.h"
#include "sampleShared/SimpleDOM.h"
#include "externalAPI/i_testbed.h"
#include <stdlib.h>

using std::string;

cQueueManager cPackHunterBehaviour::_queueManager;
tSigned32 cPackHunterBehaviour::_timeSincePathToGoal = 0;
std::vector<cPosition> cPackHunterBehaviour::_interceptPositions;


cPackHunterBehaviour::cPackHunterBehaviour(const cSimpleDOM& element, cGameObject& controlledObject) :
    _controlledObject(controlledObject),
    _queueEntry(_queueManager, element.attributeAsLong("queueCollisionShape"), controlledObject.getAgent())
{
    cGameObject* target = controlledObject.getTarget();
    assertD(target);
    _interceptPositions.push_back(target->getPosition());

    _debugDisplay = element.attributeAsBoolWithDefault("debugDisplay", false);

    _currentPath = 0;
    _timeSincePathQuery = 0;
}

cPackHunterBehaviour::~cPackHunterBehaviour()
{
    _interceptPositions.pop_back();
    delete _currentPath;
}

bool
cPackHunterBehaviour::update(cGameObject& controlledObject)
{
    cGameObject* target = controlledObject.getTarget();
    if(!target)
    {
        return true; // failed
    }

// update intercept positions if these are too old

    if(_queueEntry.isLeader())
        // so that timer will update roughly once per frame
        // also this entry's distance to target is used below
    {
        _timeSincePathToGoal++;
        if(_timeSincePathToGoal >= 6)
        {
            cGameObject* goal = gGameState->findObject("goal");
            iPath* pathToGoal = target->findShortestPathTo(goal->getPosition());
            if(pathToGoal)
            {
                float interceptDistance = _queueEntry.getDistanceToTarget() * 0.6f;
                if(interceptDistance > 700.f)
                {
                    interceptDistance = 700.f;
                }
                iShape* targetShape = target->getShape();
                tSigned32 i;
                for(i = 0; i < static_cast<tSigned32>(_interceptPositions.size()); ++i)
                {
                    float precisionX, precisionY;
                    pathToGoal->advanceAlong(targetShape, interceptDistance, gGameState->getObstructionsContext(), precisionX, precisionY);
                    _interceptPositions[i] = pathToGoal->position(0);
                }
                delete pathToGoal;
            }
            else
            {
                tSigned32 i;
                for(i = 0; i < static_cast<tSigned32>(_interceptPositions.size()); ++i)
                {
                    _interceptPositions[i] = target->getPosition();
                }
            }
            _timeSincePathToGoal = 0;
        }
    }

// update position in pack order
    _queueEntry.updateDistanceToTarget(controlledObject.distanceTo(*target));

    _timeSincePathQuery++;

    float speedMultiplier = 1.f;

    if(controlledObject.distanceTo(*target) < 320.f)
    {
        iPath* straightLinePath = controlledObject.findStraightLinePathTo(target->getPosition());
        if(straightLinePath)
        {
            _timeSincePathQuery = 0;
            delete _currentPath;
            _currentPath = straightLinePath;
            speedMultiplier = 1.4f;
        }
    }

    if(_timeSincePathToGoal > 0 && _timeSincePathQuery >= 5
     && (_currentPath == 0 || _currentPath->position(_currentPath->size() - 1) != _interceptPositions[_queueEntry.queueIndex()])
     )
    {
        _timeSincePathQuery = 0;
        delete _currentPath;
        _currentPath = controlledObject.findShortestPathTo(_interceptPositions[_queueEntry.queueIndex()]);
    }

    if(_currentPath)
    {
        if(_queueEntry.testQueueCollision())
        {
            speedMultiplier = 0.7f;
        }
        bool blocked = controlledObject.advanceAlongPath(_currentPath, speedMultiplier);
        if(blocked)
        {
            delete _currentPath;
            _currentPath = 0;
        }
    }
    else
    {
        controlledObject.faceTowards(target->getPosition());
    }
    return false;
}

void
cPackHunterBehaviour::draw(cGameObject& controlledObject)
{
    if(!_debugDisplay)
    {
        return;
    }
    if(_currentPath)
    {
        if(_queueEntry.isLeader())
        {
            gTestBed->setColour("orange");
        }
        else
        {
            gTestBed->setColour("green");
        }
        gTestBed->drawPath(_currentPath);
    }
}
