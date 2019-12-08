//**********************************************************************
//
// Copyright (c) 2002-2004
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

#include "externalAPI/i_pathengine.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

class cFileOutputStream : public iOutputStream
{
    FILE* _file;

public:

    cFileOutputStream(const char* name)
    {
        _file = fopen(name, "wb");
        if(!_file)
        {
            char buffer[20];
            sprintf(buffer, "%i", errno);
            const char* attributes[5];
            attributes[0] = "fileName";
            attributes[1] = name;
            attributes[2] = "errno";
            attributes[3] = buffer;
            attributes[4] = 0;
            Error("NonFatal", "Could not open file for writing. Nothing will be written.", attributes);
        }
    }
    ~cFileOutputStream()
    {
        if(_file)
        {
            int errorOccurred = fclose(_file);
            assertR(!errorOccurred);
        }
    }
    void put(const char* data, tUnsigned32 dataSize)
    {
        if(_file)
        {
            size_t written = fwrite(data, 1, dataSize, _file);
            assertR(written == dataSize);
            //int flushResult = fflush(_file);
            //assertR(flushResult == 0);
        }
    }
};


