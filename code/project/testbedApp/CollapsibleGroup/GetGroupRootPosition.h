
#include <vector>

class iMesh;
class iCollisionContext;
class iShape;
class cPosition;

void
GetGroupRootPosition(
        const iMesh* mesh,
        const iCollisionContext* context,
        iShape* shape,
        const std::vector<cPosition>& groupPositions,
        cPosition& result
        );
