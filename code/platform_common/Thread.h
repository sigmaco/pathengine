//**********************************************************************
//
// Copyright (c) 2013
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

#include <windows.h>

class iThread
{
public:
    virtual ~iThread() {}
    virtual void run() = 0;    
};

static DWORD WINAPI
ThreadProc(LPVOID threadDataPtr)
{
    iThread* thread = (iThread*)threadDataPtr;
    thread->run();
    return 0;
}

typedef HANDLE tThreadHandle;

static tThreadHandle
StartThread(iThread& thread)
{
    LPVOID threadDataPtr = (LPVOID)(&thread);
    return CreateThread(0, 0, ThreadProc, threadDataPtr, 0, 0);    
}

static void
JoinThread(tThreadHandle handle)
{
    DWORD result = WaitForSingleObject(handle, INFINITE);
    assertD(result == WAIT_OBJECT_0);
}

static void
Sleep_Milliseconds(tSigned32 milliseconds)
{
    Sleep(milliseconds);
}
