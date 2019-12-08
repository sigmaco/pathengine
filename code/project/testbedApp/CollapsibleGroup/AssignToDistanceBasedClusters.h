
#include <vector>

class cPosition;

void
AssignToDistanceBasedClusters(
        const std::vector<cPosition>& positions,
        tSigned32 maximumClusterRange,
        std::vector<tSigned32>& assignments
        );

void
AssignOneDistanceBasedCluster(
        const std::vector<cPosition>& positions,
        tSigned32 maximumClusterRange,
        std::vector<tSigned32>& assignedPositions,
        std::vector<tSigned32>& notAssignedPositions
        );
