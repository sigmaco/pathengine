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

class iPathEngine;
class iTestBed;

void TestbedApplicationMain(iPathEngine *pathEngine, iTestBed *testbed);

extern "C"
{

__declspec(dllexport) void __stdcall
TestApplicationEntryPoint(iPathEngine *pathEngine, iTestBed *testbed)
{
    TestbedApplicationMain(pathEngine, testbed);
}

} // extern "C"
