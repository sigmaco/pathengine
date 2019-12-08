#include "base/Header.h"
#include "project/testbedApp/CollapsibleGroup/AssignTargets.h"
#include "externalAPI/i_pathengine.h"

static double
DistanceSquared(const cPosition& p1, const cPosition& p2)
{
    double dx = static_cast<double>(p1.x) - p2.x;
    double dy = static_cast<double>(p1.y) - p2.y;
    return dx * dx + dy * dy;
}

void
AssignTargets(
        const std::vector<cPosition>& currentPositions,
        const std::vector<cPosition>& targets,
        std::vector<tSigned32>& assignments
        )
{
    assertD(!targets.empty());
    tSigned32 n = SizeL(currentPositions);
    assignments.resize(n);
    for(tSigned32 i = 0; i != n; ++i)
    {
        assignments[i] = 0;
        double bestDistance = DistanceSquared(currentPositions[i], targets[0]);
        for(tSigned32 j = 1; j != SizeL(targets); ++j)
        {
            double candidateDistance = DistanceSquared(currentPositions[i], targets[j]);
            if(candidateDistance < bestDistance)
            {
                assignments[i] = j;
                bestDistance = candidateDistance;
            }
        }
    }
}
