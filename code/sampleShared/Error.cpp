#include "base/Header.h"
#include "sampleShared/Error.h"
#include "platform_common/Process.h"
#include "platform_common/ReportErrorMessage.h"
#include <string>
#include <string.h>

typedef std::string tString;

static tString
LineToString(tSigned32 value)
{
    assertD(value >= 0);
    tSigned32 divisor = 1000000000L;
    tSigned32 quotient = value / divisor;
    while(quotient == 0)
    {
        divisor /= 10;
        quotient = value / divisor;
    }
    tString result;
    result.push_back(static_cast<char>('0' + quotient));
    value -= divisor * quotient;
    divisor /= 10;
    while(divisor)
    {
        quotient = value / divisor;
        result.push_back(static_cast<char>('0' + quotient));
        value -= divisor * quotient;
        divisor /= 10;
    }
    return result;
}

extern "C"
{

int*
PathEngine_FullAssertsOn()
{
    static int value = false;
    return &value;
}

int
PathEngine_HandleAssertion(const char *file, tSigned32 line, const char *comment)
{
    tString s(file);
    s.append("(");
    s.append(LineToString(line));
    s.append("): Assert(");
    s.append(comment);
    s.append(").");
    Error("Assertion", s.c_str(), 0);
    return 1;
}

} // #extern "C"

void
Error(const char* errorType, const char* errorString, const char *const* attributes)
{
    tString message;
    message.append("Error type: ");
    message.append(errorType);
    message.append("\n");
    message.append("Error string: ");
    message.append(errorString);
    message.append("\n");
    if(attributes && *attributes)
    {
        message.append("Error attributes:");
        message.append("\n");
        do
        {
            message.append(" ");
            message.append(attributes[0]);
            message.append(" = ");
            message.append(attributes[1]);
            message.append("\n");
            attributes += 2;
        }
        while(*attributes);
    }

    bool debugBreak;
    ReportErrorMessage(message.c_str(), "PathEngine error", debugBreak);
    if(debugBreak)
    {
        Platform_DebugBreak();
    }
    if(strcmp(errorType, "Fatal") == 0)
    {
        Platform_Abort();
    }
}
