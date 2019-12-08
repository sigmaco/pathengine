#include "base/Header.h"
#include "project/testbedApp/CollapsibleGroup/AssignToDistanceBasedClusters.h"
#include "externalAPI/i_pathengine.h"

static double
DistanceSquared(const cPosition& p1, const cPosition& p2)
{
    double dx = static_cast<double>(p1.x) - p2.x;
    double dy = static_cast<double>(p1.y) - p2.y;
    return dx * dx + dy * dy;
}

class cCluster
{
    tSigned32 _maxRange;
    bool _rangeSet;
    tSigned32 _minX, _minY, _maxX, _maxY;

public:

    cCluster(tSigned32 maxRange)
    {
        _maxRange = maxRange;
        _rangeSet = false;
    }

    bool
    add(const cPosition& position)
    {
        if(!_rangeSet)
        {
            _minX = _maxX = position.x;
            _minY = _maxY = position.y;
            _rangeSet = true;
            return true;
        }
        tSigned32 newMinX = _minX;
        tSigned32 newMinY = _minY;
        tSigned32 newMaxX = _maxX;
        tSigned32 newMaxY = _maxY;
        if(position.x < newMinX)
        {
            newMinX = position.x;
        }
        if(position.x > newMaxX)
        {
            newMaxX = position.x;
        }
        if(position.y < newMinY)
        {
            newMinY = position.y;
        }
        if(position.y > newMaxY)
        {
            newMaxY = position.y;
        }
        if(newMaxX > newMinX + _maxRange)
        {
            return false;
        }
        if(newMaxY > newMinY + _maxRange)
        {
            return false;
        }
        _minX = newMinX;
        _minY = newMinY;
        _maxX = newMaxX;
        _maxY = newMaxY;
        return true;
    }
};

void
AssignToDistanceBasedClusters(
        const std::vector<cPosition>& positions,
        tSigned32 maximumClusterRange,
        std::vector<tSigned32>& assignments
        )
{
    assignments.resize(0);
    assignments.resize(SizeL(positions), -1);
    tSigned32 numberOfAgentsAssigned = 0;
    tSigned32 nextClusterIndex = 0;
    while(numberOfAgentsAssigned < SizeL(positions))
    {
        cCluster cluster(maximumClusterRange);
        for(tSigned32 i = 0; i != SizeL(positions); ++i)
        {
            if(assignments[i] != -1)
            {
              // this position is already assigned to a cluster
                continue;
            }
            if(cluster.add(positions[i])) // we depend on this always succeeding for the first position added
            {
                assignments[i] = nextClusterIndex;
                ++numberOfAgentsAssigned;
            }
        }
        ++nextClusterIndex;
    }
}

void
AssignOneDistanceBasedCluster(
        const std::vector<cPosition>& positions,
        tSigned32 maximumClusterRange,
        std::vector<tSigned32>& assignedPositions,
        std::vector<tSigned32>& notAssignedPositions
        )
{
    assignedPositions.resize(0);
    notAssignedPositions.resize(0);
    cCluster cluster(maximumClusterRange);
    for(tSigned32 i = 0; i != SizeL(positions); ++i)
    {
        if(cluster.add(positions[i]))
        {
            assignedPositions.push_back(i);
        }
        else
        {
            notAssignedPositions.push_back(i);
        }
    }
}

