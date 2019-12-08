//**********************************************************************
//
// Copyright (c) 2006
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

#include <windows.h>

inline void
Platform_DebugBreak()
{
    DebugBreak();
}
inline void
Platform_Abort()
{
    _exit(1);
}


