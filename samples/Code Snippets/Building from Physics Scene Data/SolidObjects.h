#ifndef SOLID_OBJECTS_INCLUDED
#define SOLID_OBJECTS_INCLUDED

#include "i_pathengine.h"
#include <vector>

class cCoordinateMapping;

class cSolidObjects : public iSolidObjects
{
    std::vector<tSigned32*> _bufferPointers;

public:

    ~cSolidObjects();

    void add(const std::vector<tSigned32>& pointCoords);    

    void applyCoordinateMapping(const cCoordinateMapping& mapping);

// iSolidObjects interface

    tSigned32 numberOfConvexSolids() const;
    tSigned32 convexSolid_Points(tSigned32 convexSolidIndex) const ;
    void convexSolid_Point(tSigned32 convexSolidIndex, tSigned32 pointIndex, tSigned32& x, tSigned32& y, tSigned32& z) const;
    tSigned32 convexSolid_Attribute(tSigned32 convexSolidIndex, tSigned32 attributeIndex) const;
};

#endif
