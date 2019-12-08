#include "base/Header.h"
#include "project/testbedApp/PlayableDemo/PathFrequency.h"
#include "project/testbedApp/PlayableDemo/GameObject.h"
#include "externalAPI/i_pathengine.h"

tSigned32
GetPathFrequency(const cGameObject& controlledObject, const cGameObject& targetObject)
{
    tSigned32 distance = static_cast<tSigned32>(controlledObject.distanceTo(targetObject));
    if(distance <= 100)
    {
        // pathfind every frame at close range
        return 1;
    }
    tSigned32 result = distance - 100;
    result /= 8;
    if(result > 50)
    {
        // maximum time between paths
        result = 50;
    }
    return result;
}

tSigned32
GetPathFrequency(iPath* currentPath)
{
    tUnsigned32 length = currentPath->getLength();
    if(length <= 200)
    {
        // pathfind every frame at close range
        return 1;
    }
    length -= 100;
    if(length > 4000)
    {
        length = 4000;
    }
    return static_cast<tSigned32>(length) / 15;
}
