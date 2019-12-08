
#include "base/Header.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "externalAPI/i_pathengine.h"
#include "externalAPI/i_testbed.h"

void
TestbedApplicationMain(iPathEngine* pathengine, iTestBed* testbed)
{
// check if interfaces are compatible with the headers used for compilation
    if(testbed->getInterfaceMajorVersion()!=TESTBED_INTERFACE_MAJOR_VERSION
        ||
        testbed->getInterfaceMinorVersion()<TESTBED_INTERFACE_MINOR_VERSION)
    {
        testbed->reportError("Fatal","Testbed version is incompatible with headers used for compilation.");
        return;
    }
    if(pathengine->getInterfaceMajorVersion()!=PATHENGINE_INTERFACE_MAJOR_VERSION
        ||
        pathengine->getInterfaceMinorVersion()<PATHENGINE_INTERFACE_MINOR_VERSION)
    {
        testbed->reportError("Fatal","Pathengine version is incompatible with headers used for compilation.");
        return;
    }
    
    iMesh* mesh;
    {
        char* buffer;
        tUnsigned32 size;
        buffer=testbed->loadBinary("../resource/meshes/mesh1.xml",size);
        mesh=pathengine->loadMeshFromBuffer("xml", buffer, size, 0);
        testbed->freeBuffer(buffer);
    }
    testbed->setMesh(mesh);
    testbed->zoomExtents();

    bool exitRequested = false;
    while(!testbed->getKeyState("_ESCAPE") && !exitRequested)
    {
        testbed->setColourRGB(0.0f,0.0f,0.85f);
        testbed->drawMesh();
        testbed->setColour("blue");
        testbed->drawMeshEdges();
        testbed->update(exitRequested);
    }
}

