
#include "base/Header.h"
#include "sampleShared/Sliding.h"

enum eSide
{
    SIDE_CENTRE,
    SIDE_LEFT,
    SIDE_RIGHT
};

inline eSide SideOfLine(const tSigned32 *line, tSigned32 x, tSigned32 y)
{
    tSigned32 axisX=line[2]-line[0];
    tSigned32 axisY=line[3]-line[1];
    x-=line[0];
    y-=line[1];
    tSigned32 lhs,rhs;
    lhs=x*axisY;
    rhs=y*axisX;
    if(lhs==rhs)
        return SIDE_CENTRE;
    if(lhs<rhs)
        return SIDE_LEFT;
    return SIDE_RIGHT;    
}

inline void PushToLeftOfLine(const tSigned32 *line, tSigned32 &x, tSigned32 &y)
{
    tSigned32 axisX=line[2]-line[0];
    tSigned32 axisY=line[3]-line[1];
    tSigned32 absolute_x,absolute_y;

    absolute_x=axisX;
    if(absolute_x<0)
        absolute_x=-axisX;
    absolute_y=axisY;
    if(absolute_y<0)
        absolute_y=-axisY;
    
    // force rounding in axis with smaller component
    if(absolute_y>absolute_x)
    {
        if(axisY>0)
            x--;
        else
            x++;
    }
    else
    {
        if(axisX<0)
            y--;
        else
            y++;
    }
}

void SlideAgainst(const tSigned32* collidingLine, tSigned32 currentx, tSigned32 currenty, tSigned32& dx, tSigned32& dy)
{
    float dotproduct = static_cast<float>(dx * (collidingLine[2] - collidingLine[0]) + dy * (collidingLine[3] - collidingLine[1]));
    float ratio = dotproduct;
    tSigned32 axisX=collidingLine[2]-collidingLine[0];
    tSigned32 axisY=collidingLine[3]-collidingLine[1];
    float axisLengthSquared = static_cast<float>(axisX * axisX + axisY * axisY);
    ratio/=axisLengthSquared;
    dx = static_cast<tSigned32>(static_cast<float>(axisX) * ratio);
    dy = static_cast<tSigned32>(static_cast<float>(axisY) * ratio);
    tSigned32 targetx,targety;
    targetx=currentx+dx;
    targety=currenty+dy;
    if(SideOfLine(collidingLine,targetx,targety)==SIDE_RIGHT)
    {
        PushToLeftOfLine(collidingLine,targetx,targety);
        dx=targetx-currentx;
        dy=targety-currenty;
    }
}

void
SlideAgainst(const tSigned32* collidingLine, tSigned32 currentX, tSigned32 currentY, double& dx, double& dy)
{
    tSigned32 axisX = collidingLine[2] - collidingLine[0];
    tSigned32 axisY = collidingLine[3] - collidingLine[1];
    double dotproduct = dx * axisX + dy * axisY;
    double ratio = dotproduct;
    double axisLengthSquared = axisX * axisX + axisY * axisY;
    ratio /= axisLengthSquared;
    dx = axisX * ratio;
    dy = axisY * ratio;
    tSigned32 targetX, targetY;
    targetX = currentX + static_cast<tSigned32>(dx);
    targetY = currentY + static_cast<tSigned32>(dy);
    if(SideOfLine(collidingLine, targetX, targetY) == SIDE_RIGHT)
    {
        tSigned32 pushedTargetX = targetX;
        tSigned32 pushedTargetY = targetY;
        PushToLeftOfLine(collidingLine, pushedTargetX, pushedTargetY);
        dx += pushedTargetX - targetX;
        dy += pushedTargetY - targetY;
    }
}
