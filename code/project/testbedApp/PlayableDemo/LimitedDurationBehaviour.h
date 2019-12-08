
#include "project/testbedApp/PlayableDemo/iBehaviour.h"

class cSimpleDOM;
class cGameObject;

class cLimitedDurationBehaviour : public iBehaviour
{
    iBehaviour* _base;
    tSigned32 _remainingDuration;

public:

    cLimitedDurationBehaviour(const cSimpleDOM& element, cGameObject& controlledObject);
    bool update(cGameObject& controlledObject);
};

