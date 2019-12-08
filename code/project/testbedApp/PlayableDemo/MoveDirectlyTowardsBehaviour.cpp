#include "base/Header.h"
#include "project/testbedApp/PlayableDemo/MoveDirectlyTowardsBehaviour.h"
#include "project/testbedApp/PlayableDemo/GameState.h"
#include "project/testbedApp/PlayableDemo/GameObject.h"
#include "project/testbedApp/PlayableDemo/Globals.h"
#include "sampleShared/SimpleDOM.h"
#include "externalAPI/i_pathengine.h"
#include <math.h>

cMoveDirectlyTowardsBehaviour::cMoveDirectlyTowardsBehaviour(const cSimpleDOM& element, cGameObject& controlledObject)
{
    _targetName = element.getAttribute("target");
    _engageDistance = element.attributeAsFloatWithDefault("engageDistance", 0.f);
}

bool
cMoveDirectlyTowardsBehaviour::update(cGameObject& controlledObject)
{
    cGameObject* target = controlledObject.getTarget();
    if(!target)
    {
        return true; // failed
    }
    cPosition currentP = controlledObject.getPosition();
    cPosition targetP = target->getPosition();
    float ratio = controlledObject.getSpeed() / controlledObject.distanceTo(*target);
    float dx, dy;
    dx = (targetP.x - currentP.x) * ratio;
    dy = (targetP.y - currentP.y) * ratio;
    controlledObject._heading = static_cast<float>(atan2(dx, dy));
    if(_engageDistance != 0.f && controlledObject.distanceTo(*target) <= _engageDistance)
    {
        return true; // succeeded
    }
    bool collided = controlledObject.moveInDirection(dx, dy);
    return collided;    
}
