//**********************************************************************
//
// Copyright (c) 2002-2006
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
int PathEngine_HandleAssertion(const char*, tSigned32, const char*);
int* PathEngine_FullAssertsOn();
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#ifdef PATHENGINE_FULL_RELEASE
#define assertR(expr) do{}while(0)
#else
#define assertR(expr) do{static int on = true;if(on && !(expr)) on = PathEngine_HandleAssertion(__FILE__,__LINE__,#expr);}while(0)
#endif //def PATHENGINE_FULL_RELEASE

#ifdef ASSERTIONS_ON
#define assertF(expr) do{if(*PathEngine_FullAssertsOn() && !(expr)) PathEngine_HandleAssertion(__FILE__,__LINE__,#expr);}while(0)
#define assertD(expr) assertR(expr)
#define invalid() do{static int on=true; if(on) on = PathEngine_HandleAssertion(__FILE__,__LINE__,"<invalid>");}while(0)
#else
#define assertF(expr) do{}while(0)
#define assertD(expr) do{}while(0)
#define invalid() do{}while(0)
#endif

#define assertC(expr) do{typedef char tCompileTimeAssert[(expr)?1:-1];assertD(expr);}while(0)

#else // __cplusplus

#ifdef PATHENGINE_FULL_RELEASE
#define assertR(expr) do{}while(0)
#else
#define assertR(expr) do{if(!(expr)) PathEngine_HandleAssertion(__FILE__,__LINE__,#expr);}while(0)
#endif

#ifdef ASSERTIONS_ON
#define assertF(expr) do{if(*PathEngine_FullAssertsOn() && !(expr)) PathEngine_HandleAssertion(__FILE__,__LINE__,#expr);}while(0)
#define assertD(expr) assertR(expr)
#define invalid() do{PathEngine_HandleAssertion(__FILE__,__LINE__,"<invalid>");}while(0)
#else
#define assertF(expr) do{}while(0)
#define assertD(expr) do{}while(0)
#define invalid() do{}while(0)
#endif

#endif // __cplusplus

