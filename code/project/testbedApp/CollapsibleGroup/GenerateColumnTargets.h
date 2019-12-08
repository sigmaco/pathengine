#ifndef GENERATE_COLUMN_TARGETS_INCLUDED
#define GENERATE_COLUMN_TARGETS_INCLUDED

#include <vector>

class iMesh;
class iShape;
class iCollisionContext;
class cPosition;

void
GenerateColumnTargets(
        const iMesh* mesh, 
        iShape* agentShape, 
        const iCollisionContext* context,
        const cPosition& basePosition,
        tSigned32 forwardVectorX, tSigned32 forwardVectorY,
        tSigned32 spacing,
        tSigned32 maximumWidth,
        std::vector<cPosition>& leftTargets,
        std::vector<cPosition>& rightTargets
        );

#endif
