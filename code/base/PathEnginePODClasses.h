//**********************************************************************
//
// Copyright (c) 2010
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef PATHENGINE_POD_CLASSES_HAVE_BEEN_DEFINED
#define PATHENGINE_POD_CLASSES_HAVE_BEEN_DEFINED
class cPosition
{
public:
    cPosition() : cell(-1) {}
    tSigned32 x, y, cell;
    bool operator==(const cPosition& rhs) const
    {
        if(cell == -1)
            return cell == rhs.cell;
        return cell == rhs.cell && x == rhs.x && y == rhs.y;
    }
    bool operator!=(const cPosition& rhs) const
    {
        return !((*this) == rhs);
    }
};
class cHorizontalRange
{
public:
    tSigned32 minX, minY, maxX, maxY;
    bool operator==(const cHorizontalRange& rhs) const
    {
        return minX == rhs.minX && minY == rhs.minY && maxX == rhs.maxX && maxY == rhs.maxY;
    }
    bool operator!=(const cHorizontalRange& rhs) const
    {
        return !((*this) == rhs);
    }
};
#endif
