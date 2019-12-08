#ifndef COORDINATE_MAPPING_INCLUDED

#include "i_pathengine.h"
#include <algorithm>

class cCoordinateMapping
{
    int _sourceFor[3];
    int _sign[3];

public:

    cCoordinateMapping()
    {
      // default to identity mapping
        _sourceFor[0] = 0;
        _sourceFor[1] = 1;
        _sourceFor[2] = 2;
        _sign[0] = 1;
        _sign[1] = 1;
        _sign[2] = 1;
    }

    void
    swap(int axis1, int axis2)
    {
        std::swap(_sourceFor[axis1], _sourceFor[axis2]);
        std::swap(_sign[axis1], _sign[axis2]);
    }
    void
    negate(int axis)
    {
        _sign[axis] = -_sign[axis];
    }

    void
    applyTo(tSigned32* coords) const
    {
        tSigned32 result[3];
        tSigned32 i;
        for(i = 0; i != 3; ++i)
        {
            result[i] = coords[_sourceFor[i]] * _sign[i];
        }
        for(i = 0; i != 3; ++i)
        {
            coords[i] = result[i];
        }
    }
};

#endif
