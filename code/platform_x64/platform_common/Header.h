//**********************************************************************
//
// Copyright (c) 2002
// Thomas Young, PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

// disable warning 'symbol truncated in debugging information'
#pragma warning(disable : 4786)

// disable warning 'return type for 'nList_Internal::const_iterator<class cShape *>::operator ->' is 'class cShape **const  ' (ie; not a UDT or reference to a UDT.  Will produce errors if applied using infix notation)'
#pragma warning(disable : 4284)

// disable warning 'obsolete declaration style: please use 'cMeshWithTrackedTransform<tMesh>::~cMeshWithTrackedTransform' instead'
#pragma warning(disable : 4812)

// disable warning 'strcpy' was declared depreciated
#pragma warning(disable : 4996)

// workaround "warning C4985: 'ceil': attributes not present on previous declaration.", which is apparantly due to a bug in VS 2008
#pragma warning (disable: 4985)

typedef short tSigned16;
typedef unsigned short tUnsigned16;
typedef long tSigned32;
typedef unsigned long tUnsigned32;
typedef __int64 tSigned64;
typedef unsigned __int64 tUnsigned64;

typedef tSigned32 tSigned;
typedef tUnsigned32 tUnsigned;
static const tUnsigned32 PE_tUnsigned_Max = 0xffffffffUL;
static const tSigned32 PE_tSigned_Max = 0x7fffffffL;
static const tSigned32 PE_tSigned_Min = -0x7fffffffL - 1;

//typedef tSigned16 tSigned;
//typedef tUnsigned16 tUnsigned;
//static const tUnsigned32 PE_tUnsigned_Max = 0xffff;
//static const tSigned32 PE_tSigned_Max = 0x7fff;
//static const tSigned32 PE_tSigned_Min = -0x7fff - 1;

static const tSigned32 PE_tSigned32_Max = 0x7fffffffL;
static const tSigned32 PE_tSigned32_Min = -0x7fffffffL - 1;
static const tSigned64 PE_tSigned64_Max = 9223372036854775807LL;
static const tSigned64 PE_tSigned64_Min = (-9223372036854775807LL - 1);
static const tUnsigned64 PE_tUnsigned64_Max = 0xffffffffffffffffULL;

#define INLINE inline
#define FORCE_INLINE __forceinline
//#define DECLARE_NON_ALIASING __restrict
#define DECLARE_NON_ALIASING

template <class T> tSigned32
SizeL(const T& container)
{
    return static_cast<tSigned32>(container.size());
}
