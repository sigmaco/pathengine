
#ifndef QUEUEING_INCLUDED
#define QUEUEING_INCLUDED

#include <vector>

class iAgent;
class iCollisionContext;

class cQueueEntry;
class cQueueManager
{
    std::vector<cQueueEntry*> _order;

public:
    friend class cQueueEntry;

    tSigned32 add(cQueueEntry*);
    void remove(tSigned32);

};

class cQueueEntry
{
    cQueueManager& _manager;
    tSigned32 _queueIndex;
    float _distanceToTarget;
    iAgent* _controlledObjectAgent;
    iCollisionContext* _queueCollisionContext;
    iAgent* _queueCollisionAgent;

    void updateQueueAgentPosition();

public:

    cQueueEntry(cQueueManager& manager, tSigned32 shapeIndex, iAgent* controlledObjectAgent);
    ~cQueueEntry();

    void indexChangedTo(tSigned32 value)
    {
        _queueIndex = value;
    }

    bool isLeader() const
    {
        return _queueIndex == 0;
    }
    float getDistanceToTarget() const
    {
        return _distanceToTarget;
    }
    tSigned32 queueIndex() const
    {
        return _queueIndex;
    }

    void updateDistanceToTarget(float value);
    bool testQueueCollision();
};

#endif
