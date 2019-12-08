
#include <vector>

class cPosition;

void
AssignTargets(
        const std::vector<cPosition>& currentPositions,
        const std::vector<cPosition>& targets,
        std::vector<tSigned32>& assignments
        );
