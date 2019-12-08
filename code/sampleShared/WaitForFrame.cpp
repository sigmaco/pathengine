
#include "base/Header.h"
#include "sampleShared/WaitForFrame.h"
#include "platform_common/Timer.h"

static cTimerValue gTimeLastFrame;
static bool gFirstFrame = true;

bool
WaitForFrame()
{
    if(gFirstFrame)
    {
        gFirstFrame = false;
        gTimeLastFrame.read();
        return false;
    }

    bool overran = true;
    while(1)
    {
        cTimerValue timeNow;
        timeNow.read();
        cTimerValue elapsed = timeNow;
        elapsed -= gTimeLastFrame;
        if(elapsed.asSeconds() > 1.0 / 25.0)
        {
            gTimeLastFrame = timeNow;
            return overran;
        }
        overran = false;
    }
}
