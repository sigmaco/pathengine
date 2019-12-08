
#include "project/testbedApp/PlayableDemo/iBehaviour.h"
#include "project/testbedApp/PlayableDemo/Queueing.h"
#include "externalAPI/i_pathengine.h"
#include <vector>

class cSimpleDOM;
class cGameObject;
class iPath;
class iCollisionContext;
class iAgent;

class cPackHunterBehaviour : public iBehaviour
{
    static cQueueManager _queueManager;
    static tSigned32 _timeSincePathToGoal;
    static std::vector<cPosition> _interceptPositions;

    cGameObject& _controlledObject;
    cQueueEntry _queueEntry;
    iPath* _currentPath;
    tSigned32 _timeSincePathQuery;

    bool _debugDisplay;

public:

    cPackHunterBehaviour(const cSimpleDOM& element, cGameObject& controlledObject);
    ~cPackHunterBehaviour();

// interface for iController

    bool update(cGameObject& controlledObject);

    void draw(cGameObject& controlledObject);
};
