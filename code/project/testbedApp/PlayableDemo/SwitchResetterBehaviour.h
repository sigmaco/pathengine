
#include "project/testbedApp/PlayableDemo/iBehaviour.h"
#include "externalAPI/i_pathengine.h"
#include <vector>

class cSimpleDOM;
class cGameObject;
class iAgent;

class cSwitchResetterBehaviour : public iBehaviour
{    
    tSigned32 _radius1, _radius3;
    tSigned32 _frightenedSound;
    std::vector<cGameObject*> _targets;
    iAgent* _runAwayArea;
    iCollisionContext* _runAwayContext;
    iAgent* _avoidArea;

    enum eState
    {
        IDLE,
        RUNNING_AWAY,
        MOVING_TO_SWITCHES,
    } _state;
    cGameObject* _currentTarget;
    iPath* _currentPath;
    tSigned32 _timeSincePathQuery;

    cGameObject* chooseRandomTarget() const;

public:

    cSwitchResetterBehaviour(const cSimpleDOM& element, cGameObject& controlledObject);
    ~cSwitchResetterBehaviour();

// interface for iController

    bool update(cGameObject& controlledObject);
};
