#include "base/Header.h"
#include "sampleShared/SelectAndMove.h"
#include "base/Geometry/HorizontalRange_Helper.h"
#include "externalAPI/i_testbed.h"
//#include "externalAPI/i_pathengine.h"
#include <string.h>

static void
DrawRectangle2D(iTestBed& testbed, tSigned32 corner1X, tSigned32 corner1Y, tSigned32 corner2X, tSigned32 corner2Y)
{
    testbed.drawLine2D(corner1X, corner1Y, corner2X, corner1Y);
    testbed.drawLine2D(corner1X, corner1Y, corner1X, corner2Y);
    testbed.drawLine2D(corner2X, corner2Y, corner2X, corner1Y);
    testbed.drawLine2D(corner2X, corner2Y, corner1X, corner2Y);
}

void
cSelectAndMove::update(iTestBed& testbed, bool& completed)
{
    completed = false;
    if(!_active)
    {
        return;
    }
  // draw selection box
    if(testbed.getLeftMouseState())
    {
      // still selecting a region - draw current select region
        testbed.setColour("green");
        tSigned32 x, y;
        testbed.getMouseScreenPosition(x, y);
        DrawRectangle2D(testbed, _startX, _startY, x, y);
        return;
    }
    completed = true;
    _active = false;
}

void
cSelectAndMove::updateSelectionSet(
        iTestBed& testbed, 
        std::vector<tSigned32>& agentScreenPositions,
        std::vector<bool>& agentSelected,
        std::vector<tSigned32>& selectedAgents
        ) const
{
    assertD(!_active); // should only be called just after update method returns completed

    tSigned32 agents = SizeL(agentSelected);
    assertD(SizeL(agentScreenPositions) == agents * 2);

    cHorizontalRange range;
    InitialiseRange(range, _startX, _startY);
    tSigned32 x, y;
    testbed.getMouseScreenPosition(x, y);
    ExtendRange(range, x, y);
    while(range.maxX - range.minX < 5)
    {
        ++range.maxX;
        --range.minX;
    }
    while(range.maxY - range.minY < 5)
    {
        ++range.maxY;
        --range.minY;
    }

    selectedAgents.clear();
    for(tSigned32 i = 0; i != agents; ++i)
    {
        tSigned32 screenX = agentScreenPositions[i * 2];
        tSigned32 screenY = agentScreenPositions[i * 2 + 1];
        bool inRange = RangeContainsPoint_Inclusive(range, screenX, screenY);
        if(testbed.getKeyState("_SHIFT"))
        {
          // add to selection selection
            agentSelected[i] = agentSelected[i] || inRange;
        }
        else
        //if(testbed.getKeyState("_CONTROL")) // ** used for camera control
        if(testbed.getKeyState("T"))
        {
          // toggle selection
            if(inRange)
            {
                agentSelected[i] = !agentSelected[i];
            }
        }
        else
        {
          // set new selection set
            agentSelected[i] = inRange;
        }
        if(agentSelected[i])
        {
            selectedAgents.push_back(i);
        }
    }
}

void
cSelectAndMove::handleInputMessage(iTestBed& testbed, const char* keyPressed, bool& messageHandled, bool& moveRequested)
{
    messageHandled = false;
    if(keyPressed[0] != 'd' || keyPressed[1] != '_')
    {
        return;
    }
    if(!strcmp("LMOUSE", keyPressed + 2) && !testbed.getKeyState("_CONTROL")) // ** control plus left mouse is used for camera movement
    {
        if(!_active)
        {
            testbed.getMouseScreenPosition(_startX, _startY);
            _active = true;
        }
        messageHandled = true;
        moveRequested = false;
        return;
    }
    if(!strcmp("RMOUSE", keyPressed + 2) && !_active  && !testbed.getKeyState("_CONTROL")) // ** control plus right mouse is used to toggle full screen
    {
        messageHandled = true;
        moveRequested = true;
        return;
    }
}
