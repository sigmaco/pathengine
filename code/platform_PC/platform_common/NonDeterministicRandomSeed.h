//**********************************************************************
//
// Copyright (c) 2012
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

inline tUnsigned32
NonDeterministicRandomSeed()
{
    tUnsigned32 _high,_low;
    __asm
    {
        RDTSC
        mov _high, edx
        mov _low, eax
    }
    return _low;
}
