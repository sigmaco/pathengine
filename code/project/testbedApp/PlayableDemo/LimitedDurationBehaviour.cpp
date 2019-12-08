#include "base/Header.h"
#include "project/testbedApp/PlayableDemo/LimitedDurationBehaviour.h"
#include "sampleShared/SimpleDOM.h"
#include <stdlib.h>

cLimitedDurationBehaviour::cLimitedDurationBehaviour(const cSimpleDOM& element, cGameObject& controlledObject)
{
    _base = iBehaviour::create_IgnoreDuration(element, controlledObject);
    _remainingDuration = element.attributeAsLong("duration");
    tSigned32 variance = element.attributeAsLongWithDefault("durationVariance", 0);
    if(variance > 0)
    {
        tSigned32 adjustment = rand() % (variance * 2);
        adjustment -= variance;
        _remainingDuration += adjustment;
    }
    if(_remainingDuration < 0)
    {
        _remainingDuration = 0;
    }
}
bool
cLimitedDurationBehaviour::update(cGameObject& controlledObject)
{
    if(_remainingDuration == 0)
    {
        return true;
    }
    --_remainingDuration;
    return _base->update(controlledObject);
}
