//**********************************************************************
//
// Copyright (c) 2002-2005
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

#include <windows.h>
#include <sys/timeb.h>
#include <time.h>

#ifdef __cplusplus

class cTimerValue
{
    LARGE_INTEGER _value;

public:

    cTimerValue()
    {
    }

    void
    read()
    {
        QueryPerformanceCounter(&_value);
    }
    void
    operator-=(const cTimerValue& rhs)
    {
        _value.QuadPart -= rhs._value.QuadPart;
    }
    double
    asSeconds() const
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        return static_cast<double>(_value.QuadPart) / static_cast<double>(frequency.QuadPart);
    }
};

static const tSigned32 TIMESTAMP_BYTE_SIZE = 6;
inline void GetTimeStamp(char* buffer)
{
    struct __timeb64 tb;
    float milliseconds;
    tUnsigned32 afterPoint, seconds;
    _ftime64(&tb);
    // seconds since midnight (00:00:00), January 1, 1970, coordinated universal time (UTC)
    milliseconds = static_cast<float>(tb.millitm);
    afterPoint = static_cast<tUnsigned32>(milliseconds * 65.536f);
    //UnsignedToByteBuffer(afterPoint, 2, buffer);
    {
        unsigned short asShort = static_cast<unsigned short>(afterPoint);
        assertD(afterPoint == asShort);
        *(unsigned short*)buffer = asShort;
    }
    seconds = static_cast<tUnsigned32>(tb.time);
    //UnsignedToByteBuffer(seconds, 4, buffer + 2);
    {
        *(tUnsigned32*)(buffer + 2) = seconds;
    }
}

#endif // #ifdef __cplusplus

