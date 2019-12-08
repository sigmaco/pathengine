//**********************************************************************
//
// Copyright (c) 2010
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include "platform_common/Header.h"
#include "base/Assert.h"

void Error(const char* errorType, const char* errorString, const char *const* attributes = 0);

template <class T> tUnsigned32
SizeUL(const T& container)
{
    return static_cast<tUnsigned32>(container.size());
}
