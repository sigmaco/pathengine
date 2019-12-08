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

#include <windows.h>

INLINE const char*
ReportErrorMessage_Instructions()
{
    return "\n\nSelect 'Ok' to continue execution,\n"
                " or 'Cancel' to trigger a debug break.";
}
INLINE void
ReportErrorMessage(const char* message, const char* title, bool& debugBreak)
{
    int choice = MessageBox(0, message, title, MB_TASKMODAL | MB_OKCANCEL);
    debugBreak = (choice == IDCANCEL);
}

INLINE const char*
ReportErrorMessage_Assertion_Instructions()
{
    return "\nSelect 'Yes' to continue, and continue to report this assertion in future,\n"
                "'No' to continue, and ignore this assertion in future,\n"
                " or 'Cancel' to trigger a debug break.";
}
INLINE void
ReportErrorMessage_Assertion(const char* message, bool& debugBreak, bool& ignoreInFuture)
{
    int choice = MessageBox(0, message, "PathEngine assertion failure", MB_TASKMODAL | MB_YESNOCANCEL);
    debugBreak = (choice == IDCANCEL);
    ignoreInFuture = (choice == IDNO);
}
