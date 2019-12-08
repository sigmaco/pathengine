#include "base/Header.h"
#define extern
#include "project/testbedApp/PlayableDemo/Globals.h"
#undef extern
#include "project/testbedApp/PlayableDemo/Resources.h"
#include "project/testbedApp/PlayableDemo/GameState.h"
#include "project/testbedApp/PlayableDemo/GameObject.h"
#include "sampleShared/MoveAgent.h"
#include "sampleShared/Error.h"
#include "sampleShared/WaitForFrame.h"
#include "sampleShared/SimpleDOM.h"
#include "sampleShared/LoadWhiteSpaceDelimited.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "externalAPI/i_testbed.h"
#include "externalAPI/i_pathengine.h"
#include <vector>
#include <math.h>
#include <string.h>
#include <fstream>

using std::vector;
using std::string;

void
LoadSounds(const cSimpleDOM& sounds)
{
    string pathToSounds = "../resource/sounds/";
    size_t i;
    for(i = 0; i < sounds._children.size(); i++)
    {
        const cSimpleDOM& sound = sounds._children[i];
        if(sound._name != "sound")
        {
            continue;
        }
        string fileName = pathToSounds;
        fileName += sound.getAttribute("name");
        tSigned32 buffers = sound.attributeAsLongWithDefault("buffers", 4);
        tSigned32 index = gTestBed->createSound(fileName.c_str(), buffers);
        if(index == -1)
        {
            const char* attributes[3];
            attributes[0] = "file_name";
            attributes[1] = fileName.c_str();
            attributes[2] = 0;
            Error("Fatal", "Could not find sound file.", attributes);
        }
    }
}

string
PlayLevel(const cSimpleDOM& templates, const cSimpleDOM& script, bool onLastLevel)
{
    gTestBed->update_ExitIfRequested();

    new cGameState(templates, script); // assigns self to gGameState in constructor

    tSigned32 i = script.firstChildWithName("initialisation");
    if(i >= 0)
    {
        const cSimpleDOM& initialisationScript = script._children[i];
        size_t j;
        for(j = 0; j < initialisationScript._children.size(); j++)
        {
            gGameState->execute(initialisationScript._children[j]);
        }
    }

    string result;
    while(!gGameState->levelCompleted(result))
    {
        gGameState->draw();

        WaitForFrame();

      // tell the testbed to render this frame

        gTestBed->update_ExitIfRequested();

        static bool paused = false;
        if(gTestBed->getKeyState("P"))
        {
            paused = gTestBed->getKeyState("_RSHIFT");
        }

        if(!paused)
        {
            gGameState->update();
        }

    }

    bool pressedSpace = false;
    do
    {
        if(result == "win")
        {
            if(onLastLevel)
            {
                gTestBed->displaySplashImage_Centred(cResources::SPLASH_ALLCOMPLETE);
            }
            else
            {
                gTestBed->displaySplashImage_Centred(cResources::SPLASH_LEVELCOMPLETE);
            }
        }
        else
        {
            gTestBed->displaySplashImage_Centred(cResources::SPLASH_DIED);
        }

        gGameState->draw();

        gTestBed->update_ExitIfRequested();
        const char* keyPressed;
        while(keyPressed = gTestBed->receiveKeyMessage())
        {
            if(!strcmp("d_SPACE", keyPressed))
            {
                pressedSpace = true;
            }
        }
    }
    while(!pressedSpace);

    delete gGameState;
    gGameState = 0;
    return result;
}

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


    gPathEngine = pathEngine;
    gTestBed = testBed;

    cSimpleDOM script;
    {
        std::ifstream is("../resource/scripts/demo_script.txt");
        if(!is.good())
        {
            Error("Fatal", "Could not open demo script ('resource/scripts/demo_script.txt').");
        }
        LoadWhiteSpaceDelimited(is, script);
    }
    assertR(script._name == "script");

    tSigned32 child = script.firstChildWithName("sounds");
    if(child >= 0)
    {
        LoadSounds(script._children[child]);
    }

    cResources resources(script.refFirstChildWithName("shapes"));
    gResources = &resources;

    cSimpleDOM& templates = script.refFirstChildWithName("templates");
    cSimpleDOM& levels = script.refFirstChildWithName("levels");

    assertR(!levels._children.empty());

    {

        bool pressedSpace = false;
        do
        {
            gTestBed->displaySplashImage_Centred(cResources::SPLASH_BEGIN);
            gTestBed->update_ExitIfRequested();
            const char* keyPressed;
            while(keyPressed = gTestBed->receiveKeyMessage())
            {
                if(!strcmp("d_SPACE", keyPressed))
                {
                    pressedSpace = true;
                }
            }
        }
        while(!pressedSpace);
    }


    size_t i = 0;
    while(1)
    {
        bool onLastLevel = (i + 1 == levels._children.size());
        string result = PlayLevel(templates, levels._children[i], onLastLevel);
        if(result == "win")
        {
            i++;
            if(onLastLevel)
            {
                i = 0;
            }
        }
    }
}
