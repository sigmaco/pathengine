#include "base/Header.h"
#include "project/testbedApp/SeparatingGroups/SeparatingGroups.h"
#include "sampleShared/Error.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "externalAPI/i_testbed.h"
#include "externalAPI/i_pathengine.h"

void
TestbedApplicationMain(iPathEngine* pathEngine, iTestBed* testBed)
{
// check if interfaces are compatible with the headers used for compilation
    if(testBed->getInterfaceMajorVersion()!=TESTBED_INTERFACE_MAJOR_VERSION
        ||
        testBed->getInterfaceMinorVersion()<TESTBED_INTERFACE_MINOR_VERSION)
    {
        testBed->reportError("Fatal","Testbed version is incompatible with headers used for compilation.");
        return;
    }
    if(pathEngine->getInterfaceMajorVersion()!=PATHENGINE_INTERFACE_MAJOR_VERSION
        ||
        pathEngine->getInterfaceMinorVersion()<PATHENGINE_INTERFACE_MINOR_VERSION)
    {
        testBed->reportError("Fatal","Pathengine version is incompatible with headers used for compilation.");
        return;
    }

    SeparatingGroups(*testBed, *pathEngine);
}
