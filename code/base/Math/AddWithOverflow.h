//**********************************************************************
//
// Copyright (c) 2004
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

inline bool
AddWithOverflow(tUnsigned32 source1, tUnsigned32 source2, tUnsigned32 &result)
{
    result = source1 + source2;
    return result < source1;
}
inline bool
AddWithOverflow(tSigned32 source1, tSigned32 source2, tSigned32 &result)
{
    result = source1 + source2;
    if(source2 >= 0)
    {
        return result < source1;
    }
    return result > source1;
}
inline bool
SubtractWithOverflow(tSigned32 source1, tSigned32 source2, tSigned32 &result)
{
    result = source1 - source2;
    if(source2 >= 0)
    {
        return result > source1;
    }
    return result < source1;
}

