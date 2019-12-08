#include "base/Header.h"
#include "project/testbedApp/PlayableDemo/Resources.h"
#include "project/testbedApp/PlayableDemo/Globals.h"
#include "sampleShared/SimpleDOM.h"
#include "externalAPI/i_pathengine.h"
#include "externalAPI/i_testbed.h"
#include <stdlib.h>

using std::string;

cResources::cResources(const cSimpleDOM& shapesScript)
{
    tSigned32 index;
    index = gTestBed->loadSplashImage("../resource/splashImages/demo_begin.png");
    if(index == -1)
    {
        Error("Fatal", "Failed to load splash image 'demo_begin.png'.");
    }
    assertD(index == SPLASH_BEGIN);
    index = gTestBed->loadSplashImage("../resource/splashImages/demo_died.png");
    if(index == -1)
    {
        Error("Fatal", "Failed to load splash image 'demo_died.png'.");
    }
    assertD(index == SPLASH_DIED);
    index = gTestBed->loadSplashImage("../resource/splashImages/demo_levelcomplete.png");
    if(index == -1)
    {
        Error("Fatal", "Failed to load splash image 'demo_levelcomplete.png'.");
    }
    assertD(index == SPLASH_LEVELCOMPLETE);
    index = gTestBed->loadSplashImage("../resource/splashImages/demo_allcomplete.png");
    if(index == -1)
    {
        Error("Fatal", "Failed to load splash image 'demo_allcomplete.png'.");
    }
    assertD(index == SPLASH_ALLCOMPLETE);
    index = gTestBed->loadSplashImage("../resource/splashImages/demo_camera.png");
    if(index == -1)
    {
        Error("Fatal", "Failed to load splash image 'demo_camera.png'.");
    }
    assertD(index == SPLASH_CAMERA);

    tSigned32 i;
    for(i = 0; i < static_cast<tSigned32>(shapesScript._children.size()); ++i)
    {
        const cSimpleDOM& element = shapesScript._children[i];
        if(element._name != "shape")
        {
            continue;
        }
        string verticesString = element.getAttribute("vertices");
        const char* ptr = verticesString.c_str();
        std::vector<tSigned32> coords;
        while(1)
        {
            char* nextPtr;
            coords.push_back(strtol(ptr, &nextPtr, 10));
            assertR(*nextPtr == ',');
            ptr = nextPtr + 1;
            coords.push_back(strtol(ptr, &nextPtr, 10));
            if(*nextPtr == 0)
            {
                break;
            }
            assertR(*nextPtr == ',');
            ptr = nextPtr + 1;
        }
        _shapes.push_back(gPathEngine->newShape(&coords.front(), SizeL(coords)));
    }
}

cResources::~cResources()
{
    size_t i;
    for(i = 0; i < _shapes.size(); ++i)
    {
        delete _shapes[i];
    }
}

