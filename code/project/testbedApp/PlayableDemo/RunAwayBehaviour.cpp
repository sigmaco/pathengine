#include "base/Header.h"
#include "project/testbedApp/PlayableDemo/RunAwayBehaviour.h"
#include "project/testbedApp/PlayableDemo/Globals.h"
#include "project/testbedApp/PlayableDemo/GameState.h"
#include "project/testbedApp/PlayableDemo/GameObject.h"
#include "project/testbedApp/PlayableDemo/PathFrequency.h"
#include "sampleShared/MoveAgent.h"
#include "sampleShared/SimpleDOM.h"
#include "externalAPI/i_testbed.h"
#include <math.h>
#include <stdlib.h>

using std::string;

cRunAwayBehaviour::cRunAwayBehaviour(const cSimpleDOM& element, cGameObject& controlledObject)
{
    _distanceAway = element.attributeAsLong("distanceAway");
    _currentPath = 0;
    _timeSincePathQuery = 0;
}

cRunAwayBehaviour::~cRunAwayBehaviour()
{
    delete _currentPath;
}

bool
cRunAwayBehaviour::update(cGameObject& controlledObject)
{
    cGameObject* target = controlledObject.getTarget();
    if(!target)
    {
        return true; // failed
    }
    
//    tSigned32 frequency = GetPathFrequency(controlledObject, *target);
    tSigned32 frequency = 10;

    _timeSincePathQuery++;
    if(_timeSincePathQuery >= frequency)
    {
        _timeSincePathQuery = 0;
        delete _currentPath;
        _currentPath = controlledObject.findPathAway(target->getPosition(), _distanceAway);
    }

    if(_currentPath)
    {
        float speedMultiplier = 1.f;
        if(frequency == 1)
        {
        // add a speed boost when close in so the AI can really go for the throat
            speedMultiplier = 1.15f;
        }
        bool blocked = controlledObject.advanceAlongPath(_currentPath, speedMultiplier);
        if(blocked)
        {
            delete _currentPath;
            _currentPath = 0;
        }
    }
    return false;
}
