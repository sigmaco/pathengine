#include "base/Header.h"
#include "project/testbedApp/PlayableDemo/VisitGreenSceneryBehaviour.h"
#include "project/testbedApp/PlayableDemo/GameState.h"
#include "project/testbedApp/PlayableDemo/GameObject.h"
#include "project/testbedApp/PlayableDemo/Globals.h"
#include "sampleShared/SimpleDOM.h"
#include <stdlib.h>

cVisitGreenSceneryBehaviour::cVisitGreenSceneryBehaviour(const cSimpleDOM& element, cGameObject& controlledObject)
{
    _engageDistance = element.attributeAsFloat("engageDistance");
    std::vector<cGameObject*> greenSceneryObjects;
    gGameState->findAllObjectsWithType("sceneryGreen", greenSceneryObjects);
    tSigned32 i;
    for(i = 0; i < static_cast<tSigned32>(greenSceneryObjects.size()); ++i)
    {
        cGameObject* object = greenSceneryObjects[i];
        cPosition baseP = object->getPosition();
        cPosition unobstructedP = gGameState->getMesh()->findClosestUnobstructedPosition(controlledObject.getShape(), gGameState->getObstructionsContext(), baseP, 1000);
        if(unobstructedP.cell != -1)
        {
            _greenScenery.push_back(unobstructedP);
        }
    }
    _path = 0;
}

bool
cVisitGreenSceneryBehaviour::update(cGameObject& controlledObject)
{
    if(!_path)
    {
        size_t i = rand() % _greenScenery.size();
        _path = controlledObject.findShortestPathTo(_greenScenery[i]);
        if(!_path)
        {
            return false;
        }
    }
    bool blocked = controlledObject.advanceAlongPath(_path);
    if(blocked || _path->size() == 1)
    {
        delete _path;
        _path = 0;
    }
    return false;
}
