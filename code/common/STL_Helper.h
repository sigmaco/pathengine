//**********************************************************************
//
// Copyright (c) 2002
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#pragma once

#include <algorithm>

template <class T, class T2> void
Fill(T& container, T2 value)
{
    std::fill(container.begin(),container.end(),value);
}

template <class T, class T2> bool
Contains(T& container, T2 value)
{
    return std::find(container.begin(),container.end(),value)!=container.end();
}

template <class T> void
ShrinkToFit(T& container)
{
    if(container.capacity() != container.size())
    {
        T(container).swap(container);
    }
}
template <class T> void
ShrinkToFit_Compound(T& container)
{
    T(container).swap(container);
}
template <class T> void
CompletelyClear(T& container)
{
    if(container.capacity() != 0)
    {
        T().swap(container);
    }
}

template <class T> typename T::value_type*
VectorBuffer(T& v)
{
    if(v.empty())
    {
        return 0;
    }
    return &v.front();
}
template <class T> const typename T::value_type*
VectorBuffer(const T& v)
{
    if(v.empty())
    {
        return 0;
    }
    return &v.front();
}

