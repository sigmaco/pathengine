#ifndef ROW_TARGETS_INCLUDED
#define ROW_TARGETS_INCLUDED

#include "externalAPI/i_pathengine.h"
#include <vector>

class cCollapsibleGroupAgent;

class cRowTargets
{
    std::vector<cPosition> _positions;
    std::vector<const cCollapsibleGroupAgent*> _assigned;
    tSigned32 _centreI;

public:

    cRowTargets(
        const std::vector<cPosition>& leftTargets,
        const cPosition& basePosition,
        const std::vector<cPosition>& rightTargets
        );

    tSigned32 firstColumn() const;
    tSigned32 lastColumn() const;
    bool isValidColumn(tSigned32 targetColumn) const;
    const cPosition& position(tSigned32 column) const;
    bool isAssigned(tSigned32 column) const;
    void assign(tSigned32 column, const cCollapsibleGroupAgent* agent);
    void unassign(tSigned32 column, const cCollapsibleGroupAgent* agent);
    const cCollapsibleGroupAgent* assignedAgent(tSigned32 column);

    tSigned32 closestValidColumn(tSigned32 targetColumn) const;

    tSigned32 centringAndWidthAreIdentical(const cRowTargets& rhsTargets) const;
};

#endif
