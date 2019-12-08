
#include "project/testbedApp/PlayableDemo/iBehaviour.h"
#include "project/testbedApp/PlayableDemo/Queueing.h"
#include <vector>

class cSimpleDOM;
class cGameObject;
class iPath;

class cRescueFromHuntersBehaviour : public iBehaviour
{
    static std::vector<cGameObject*> _targets;
    static std::vector<cQueueManager> _queueManagers;

    tSigned32 _queueShape;
    tSigned32 _targetIndex;
    cQueueEntry* _queueEntry;
    iPath* _currentPath;
    tSigned32 _timeSincePathQuery;

    tSigned32 findClosestTarget(cGameObject& controlledObject);

public:

    cRescueFromHuntersBehaviour(const cSimpleDOM& element, cGameObject& controlledObject);
    ~cRescueFromHuntersBehaviour();

// interface for iController

    bool update(cGameObject& controlledObject);
};
