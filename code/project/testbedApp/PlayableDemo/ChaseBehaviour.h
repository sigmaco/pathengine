
#include "project/testbedApp/PlayableDemo/iBehaviour.h"

class cSimpleDOM;
class cGameObject;
class iPath;

class cChaseBehaviour : public iBehaviour
{
    tSigned32 _activationSound;
    iPath* _currentPath;
    tSigned32 _timeSincePathQuery;
    tSigned32 _engageDistance;
    tSigned32 _findClosestUnobstructedPointRange;

public:

    cChaseBehaviour(const cSimpleDOM& element, cGameObject& controlledObject);
    ~cChaseBehaviour();

// interface for iController

    bool update(cGameObject& controlledObject);
};
