#include "sampleShared/IntegerPoint2D.h"

cIntegerPoint2D::eSide
cIntegerPoint2D::sideOf(const cIntegerPoint2D& rhs) const
{
    int multiplied1 = x * rhs.y;
    int multiplied2 = y * rhs.x;
    if(multiplied1==multiplied2)
        return SIDE_CENTRE;
    if(multiplied1<multiplied2)
        return SIDE_LEFT;
    return SIDE_RIGHT;
}

int
cIntegerPoint2D::lengthSquared() const
{
    return x * x + y * y;
}
