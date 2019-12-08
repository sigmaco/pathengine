//**********************************************************************
//
// Copyright (c) 2004-2007
// Thomas Young, PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

#ifdef PATHENGINE_SINGLE_THREADED
class cMutex
{
    // prevent copying and assignment
    cMutex(const cMutex&);
    const cMutex& operator=(const cMutex&);
public:
    cMutex()
    {
    }
    ~cMutex()
    {
    }
    void lock() const
    {
    }
    void unlock() const
    {
    }
};
#else
#include <windows.h>

class cMutex
{
    mutable CRITICAL_SECTION _cs;

    // prevent copying and assignment
    cMutex(const cMutex&);
    const cMutex& operator=(const cMutex&);

public:

    cMutex()
    {
        InitializeCriticalSection(&_cs);
    }
    ~cMutex()
    {
        DeleteCriticalSection(&_cs);
    }

    void lock() const
    {
        EnterCriticalSection(&_cs);
    }
    void unlock() const
    {
        LeaveCriticalSection(&_cs);
    }
};
#endif

