//**********************************************************************
//
// Copyright (c) 2012
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include <string.h>

inline int StrCmpI(const char *s1, const char *s2)
{
    return _stricmp(s1, s2);
}
inline int StrCmpI(const char *s1, const char *s2, tUnsigned32 n)
{
    return _strnicmp(s1, s2, n);
}
