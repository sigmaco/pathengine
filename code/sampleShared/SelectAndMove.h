#ifndef SELECT_AND_MOVE_INCLUDED
#define SELECT_AND_MOVE_INCLUDED

#include <vector>

class iTestBed;

class cSelectAndMove
{
    bool _active;
    tSigned32 _startX, _startY;

public:

    cSelectAndMove()
    {
        _active = false;
    }

    bool
    isActive() const
    {
        return _active;
    }

    // call during rendering
    void update(iTestBed& testbed, bool& completed);

    // called directly after update, if completed set to true
    // (separated out to avoid cost of building screen positions in other cases)
    void
    updateSelectionSet(
            iTestBed& testbed, 
            std::vector<tSigned32>& agentScreenPositions,
            std::vector<bool>& agentSelected, // current selection state, to be updated
            std::vector<tSigned32>& selectedAgents // updated to the set of indices flagged in agentSelected, for convenience
            ) const;
    
    void handleInputMessage(iTestBed& testbed, const char* keyPressed, bool& messageHandled, bool& moveRequested);
};

#endif
