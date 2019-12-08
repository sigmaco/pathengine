
#include "base/Header.h"
#include "sampleShared/GenerateAndSaveRandomSeed.h"
#include "platform_common/NonDeterministicRandomSeed.h"
#include "externalAPI/i_pathengine.h"
#include <fstream>

tUnsigned32
GenerateAndSaveRandomSeed()
{
    tUnsigned32 result = NonDeterministicRandomSeed();

    std::ofstream os("savedRandomSeed.txt");
    os << result;
    if(!os)
    {
        Error("NonFatal", "An error occurred saving the random seed.", 0);
    }
    return result;
}
