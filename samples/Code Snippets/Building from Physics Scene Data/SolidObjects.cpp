#include "SolidObjects.h"
#include "CoordinateMapping.h"
#include <assert.h>

cSolidObjects::~cSolidObjects()
{
    tSigned32 i;
    for(i = 0; i != static_cast<tSigned32>(_bufferPointers.size()); i += 2)
    {
        delete [] _bufferPointers[i];
    }
}

void
cSolidObjects::add(const std::vector<tSigned32>& pointCoords)
{
    //assert(!pointCoords.empty());
    if(pointCoords.empty())
    {
        return;
    }
    tSigned32* bufferStart = new tSigned32 [pointCoords.size()];
    tSigned32* bufferEnd = bufferStart + pointCoords.size();
    _bufferPointers.push_back(bufferStart);
    _bufferPointers.push_back(bufferEnd);
    for(tSigned32 i = 0; i != static_cast<tSigned32>(pointCoords.size()); ++i)
    {
        bufferStart[i] = pointCoords[i];
    }
}

void
cSolidObjects::applyCoordinateMapping(const cCoordinateMapping& mapping)
{
    for(tSigned32 i = 0; i != numberOfConvexSolids(); ++i)
    {
        for(tSigned32 j = 0; j != convexSolid_Points(i); ++j)
        {
            mapping.applyTo(_bufferPointers[i * 2] + j * 3);
        }
    }
}

// iSolidObjects interface

tSigned32
cSolidObjects::numberOfConvexSolids() const
{
    return static_cast<tSigned32>(_bufferPointers.size() / 2);
}
tSigned32
cSolidObjects::convexSolid_Points(tSigned32 convexSolidIndex) const
{
    tSigned32 i = convexSolidIndex * 2;
    return static_cast<tSigned32>((_bufferPointers[i + 1] - _bufferPointers[i]) / 3);
}
void
cSolidObjects::convexSolid_Point(tSigned32 convexSolidIndex, tSigned32 pointIndex, tSigned32& x, tSigned32& y, tSigned32& z) const
{
    assert(convexSolidIndex >= 0 && convexSolidIndex < numberOfConvexSolids());
    assert(pointIndex >= 0 && pointIndex < convexSolid_Points(convexSolidIndex));
    const tSigned32* bufferStart = _bufferPointers[convexSolidIndex * 2];
    const tSigned32* point = bufferStart + pointIndex * 3;
    x = point[0];
    y = point[1];
    z = point[2];
}
tSigned32
cSolidObjects::convexSolid_Attribute(tSigned32, tSigned32) const
{
    return -1;
}
