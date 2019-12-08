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

#include <intrin.h>

inline tUnsigned32
NonDeterministicRandomSeed()
{
    tUnsigned64 asI64 = __rdtsc();
    return (tUnsigned32)asI64;
}
