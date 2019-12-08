//**********************************************************************
//
// Copyright (c) 2007
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

#include "base/PathEnginePODClasses.h"
#include "base/Math/AddWithOverflow.h"

inline void
InitialiseRangeToIntegerLimits(cHorizontalRange& r)
{
    r.minX = r.minY = PE_tSigned32_Max;
    r.maxX = r.maxY = PE_tSigned32_Min;
}

inline void
InitialiseRange(cHorizontalRange& r, tSigned32 x, tSigned32 y)
{
    r.minX = r.maxX = x;
    r.minY = r.maxY = y;
}

inline void
InitialiseSquareRange_Clipped(cHorizontalRange& r, tSigned32 x, tSigned32 y, tSigned32 radius, tSigned32 coordinateRange)
{
    assertD(x >= -coordinateRange && x <= coordinateRange);
    assertD(y >= -coordinateRange && y <= coordinateRange);
    bool overflow;
    overflow = SubtractWithOverflow(x, radius, r.minX);
    if(overflow || r.minX < -coordinateRange)
    {
        r.minX = -coordinateRange;
    }
    overflow = AddWithOverflow(x, radius, r.maxX);
    if(overflow || r.maxX > coordinateRange)
    {
        r.maxX = coordinateRange;
    }
    overflow = SubtractWithOverflow(y, radius, r.minY);
    if(overflow || r.minY < -coordinateRange)
    {
        r.minY = -coordinateRange;
    }
    overflow = AddWithOverflow(y, radius, r.maxY);
    if(overflow || r.maxY > coordinateRange)
    {
        r.maxY = coordinateRange;
    }
}

inline void
ExtendRange(cHorizontalRange& r, tSigned32 x, tSigned32 y)
{
    if(x < r.minX)
    {
        r.minX = x;
    }
    if(x > r.maxX)
    {
        r.maxX = x;
    }
    if(y < r.minY)
    {
        r.minY = y;
    }
    if(y > r.maxY)
    {
        r.maxY = y;
    }
}

inline bool
RangesTouchOrOverlap(const cHorizontalRange& r1, const cHorizontalRange& r2)
{
    if(r1.maxX < r2.minX)
    {
        return false;
    }
    if(r1.minX > r2.maxX)
    {
        return false;
    }
    if(r1.maxY < r2.minY)
    {
        return false;
    }
    if(r1.minY > r2.maxY)
    {
        return false;
    }
    return true;
}
inline bool
RangesOverlap(const cHorizontalRange& r1, const cHorizontalRange& r2)
{
    if(r1.maxX <= r2.minX)
    {
        return false;
    }
    if(r1.minX >= r2.maxX)
    {
        return false;
    }
    if(r1.maxY <= r2.minY)
    {
        return false;
    }
    if(r1.minY >= r2.maxY)
    {
        return false;
    }
    return true;
}

inline bool
RangeContainsPoint_Inclusive(const cHorizontalRange& r, tSigned32 x, tSigned32 y)
{
    return x >= r.minX
        && x <= r.maxX
        && y >= r.minY
        && y <= r.maxY
        ;
}
inline bool
RangeContainsPoint_NonInclusive(const cHorizontalRange& r, tSigned32 x, tSigned32 y)
{
    return x > r.minX
        && x < r.maxX
        && y > r.minY
        && y < r.maxY
        ;
}

inline bool
RangeContainsRange(const cHorizontalRange& lhs, const cHorizontalRange& rhs)
{
    return RangeContainsPoint_Inclusive(lhs, rhs.minX, rhs.minY)
        && RangeContainsPoint_Inclusive(lhs, rhs.maxX, rhs.maxY)
        ;
}

inline tSigned32
CoordinateRangeUsed(const cHorizontalRange& r)
{
    tSigned32 result = r.minX;
    if(result < 0)
    {
        result = -result;
    }
    tSigned32 candidate;
    candidate = r.minY;
    if(candidate < 0)
    {
        candidate = -candidate;
    }
    if(candidate > result)
    {
        result = candidate;
    }
    candidate = r.maxX;
    if(candidate < 0)
    {
        candidate = -candidate;
    }
    if(candidate > result)
    {
        result = candidate;
    }
    candidate = r.maxY;
    if(candidate < 0)
    {
        candidate = -candidate;
    }
    if(candidate > result)
    {
        result = candidate;
    }
    return result;
}

inline void
AddRange(cHorizontalRange& addTo, const cHorizontalRange& toAdd)
{
    if(toAdd.minX < addTo.minX)
    {
        addTo.minX = toAdd.minX;
    }
    if(toAdd.minY < addTo.minY)
    {
        addTo.minY = toAdd.minY;
    }
    if(toAdd.maxX > addTo.maxX)
    {
        addTo.maxX = toAdd.maxX;
    }
    if(toAdd.maxY > addTo.maxY)
    {
        addTo.maxY = toAdd.maxY;
    }
}

inline void
GetRangeIntersection_KnownToIntersect(cHorizontalRange& dest, const cHorizontalRange& src)
{
    assertD(RangesTouchOrOverlap(src, dest));
    if(src.minX > dest.minX)
    {
        dest.minX = src.minX;
    }
    if(src.minY > dest.minY)
    {
        dest.minY = src.minY;
    }
    if(src.maxX < dest.maxX)
    {
        dest.maxX = src.maxX;
    }
    if(src.maxY < dest.maxY)
    {
        dest.maxY = src.maxY;
    }
}

inline tSigned32&
IndexedRangeMin(cHorizontalRange& range, int index)
{
    if(index == 0)
    {
        return range.minX;
    }
    return range.minY;
}
inline tSigned32&
IndexedRangeMax(cHorizontalRange& range, int index)
{
    if(index == 0)
    {
        return range.maxX;
    }
    return range.maxY;
}
inline const tSigned32&
IndexedRangeMin(const cHorizontalRange& range, int index)
{
    if(index == 0)
    {
        return range.minX;
    }
    return range.minY;
}
inline const tSigned32&
IndexedRangeMax(const cHorizontalRange& range, int index)
{
    if(index == 0)
    {
        return range.maxX;
    }
    return range.maxY;
}

// ** note that there are no range checks here **
// ** calling code needs to ensure that there is no possibility of overflow **
inline void
TranslateRange(cHorizontalRange& range, tSigned32 x, tSigned32 y)
{
    range.minX += x;
    range.minY += y;
    range.maxX += x;
    range.maxY += y;
}
