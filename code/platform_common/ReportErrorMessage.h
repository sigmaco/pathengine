//**********************************************************************
//
// Copyright (c) 2010
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

#include <stdio.h>

INLINE const char*
ReportErrorMessage_Instructions()
{
    return "";
}
INLINE void
ReportErrorMessage(const char* message, const char* title, bool& debugBreak)
{
    printf("%s", title);
    printf(":\n");
    printf("%s", message);
    printf("\n");
    debugBreak = true;
}

INLINE const char*
ReportErrorMessage_Assertion_Instructions()
{
    return "";
}
INLINE void
ReportErrorMessage_Assertion(const char* message, bool& debugBreak, bool& ignoreInFuture)
{
    ReportErrorMessage(message, "PathEngine assertion failure", debugBreak);
    ignoreInFuture = false;
}

