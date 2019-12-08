#include "LoadCP3D.h"
#include "i_pathengine.h"
#include "i_contentprocessing3d.h"
#include <windows.h>
#include <stdio.h>

class cErrorHandler : public iErrorHandler
{
    cErrorHandler(){}
public:
    eAction handle(const char* error_type, const char* error_string, const char *const* attributes)
    //eAction handle(const char*, const char*, const char *const*)
    {
        DebugBreak();
        return CONTINUE;
    }
    static cErrorHandler& refInstance()
    {
        static cErrorHandler the_instance;
        return the_instance;
    }
};

typedef void* (__stdcall* tGetInterfacesFunction)(iErrorHandler*, iPathEngine**, iContentProcessing3D**);

static void
LoadContentProcessing3D(const char* fileName, iErrorHandler* handler, iPathEngine*& pathEngine, iContentProcessing3D*& cp3d)
{
    pathEngine = 0;
    cp3d = 0;
    char buffer[1000];
    DWORD errorValue;
    HINSTANCE hInstance;
    hInstance = LoadLibrary(fileName);
    if(!hInstance)
    {
        errorValue = GetLastError();
        sprintf(buffer, "file name = '%s', GetLastError() returned %d\n** Note that the 3D content processing DLL is not included in the 'testbed only' archive **", fileName, errorValue);
        MessageBox(NULL, buffer, "Error: failed to load 3D content processing dll", MB_TASKMODAL);
        return;
    }
    FARPROC procAddress;
    SetLastError(0);
    procAddress = GetProcAddress(hInstance, (LPCSTR)2);
    if(!procAddress)
    {
        errorValue = GetLastError();
        MessageBox(NULL, fileName, "Error: Failed to obtain PathEngine 3D content processing entrypoint in", MB_TASKMODAL);
        sprintf(buffer, "%d", errorValue);
        MessageBox(NULL, buffer, "GetLastError()", MB_TASKMODAL);
        return;
    }

    tGetInterfacesFunction getInterfaceFunction = (tGetInterfacesFunction)procAddress;
    getInterfaceFunction(handler, &pathEngine, &cp3d);
}

iContentProcessing3D*
LoadCP3D()
{
    iPathEngine* pathEngine;
    iContentProcessing3D* cp3d;
    LoadContentProcessing3D("contentProcessing3D", &cErrorHandler::refInstance(), pathEngine, cp3d);
    return cp3d;
}
