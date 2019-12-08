#ifndef FORMATION_MOVEMENT_PARAMETERS_INCLUDED
#define FORMATION_MOVEMENT_PARAMETERS_INCLUDED

class cFormationMovementParameters
{
public:

    tSigned32 _agentSpacing;
    double _agentSpeed;
    tSigned32 _maximumWidth;
    tSigned32 _maximumClusterRange;
    tSigned32 _numberOfStartPositions;
    double _groupStepDistance;
    double _trailingDistanceSlowDownStart;
    double _trailingDistanceSlowDownEnd;

    cFormationMovementParameters()
    {
        _agentSpacing = 25;
        _agentSpeed = 8.f;
        _maximumWidth = 3;
        _maximumClusterRange = 500;
        _numberOfStartPositions = (_maximumWidth * 2 + 1) * 10; // first ten rows are considered as potential start positions
        _groupStepDistance = 25.f;
        _trailingDistanceSlowDownStart = 25.;
        _trailingDistanceSlowDownEnd = 100.;
    }
};

#endif
