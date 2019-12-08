
#include "project/testbedApp/PlayableDemo/iBehaviour.h"

class cSimpleDOM;
class cGameObject;
class iPath;

class cRunAwayBehaviour : public iBehaviour
{
    tSigned32 _distanceAway;
    iPath* _currentPath;
    tSigned32 _timeSincePathQuery;

public:

    cRunAwayBehaviour(const cSimpleDOM& element, cGameObject& controlledObject);
    ~cRunAwayBehaviour();

// interface for iController

    bool update(cGameObject& controlledObject);
};
