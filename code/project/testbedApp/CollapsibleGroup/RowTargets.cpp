#include "base/Header.h"
#include "project/testbedApp/CollapsibleGroup/RowTargets.h"
#include "externalAPI/i_pathengine.h"

cRowTargets::cRowTargets(
        const std::vector<cPosition>& leftTargets,
        const cPosition& basePosition,
        const std::vector<cPosition>& rightTargets
        ) :
 _positions(SizeL(leftTargets) + 1 + SizeL(rightTargets)),
 _assigned(SizeL(_positions), (const cCollapsibleGroupAgent*)0) // ** cast added here to work around strange compile error with clang choosing vector copy region constructor
{
    _centreI = SizeL(leftTargets);
    tSigned32 i;
    for(i = 0; i != SizeL(leftTargets); ++i)
    {
        _positions[_centreI - 1 - i] = leftTargets[i];
    }
    _positions[_centreI] = basePosition;
    for(i = 0; i != SizeL(rightTargets); ++i)
    {
        _positions[_centreI + 1 + i] = rightTargets[i];
    }
}

tSigned32
cRowTargets::firstColumn() const
{
    return -_centreI;
}
tSigned32
cRowTargets::lastColumn() const
{
    return SizeL(_positions) - 1 - _centreI;
}
bool
cRowTargets::isValidColumn(tSigned32 targetColumn) const
{
    return targetColumn >= firstColumn() && targetColumn <= lastColumn();
}

const cPosition&
cRowTargets::position(tSigned32 column) const
{
    assertD(isValidColumn(column));
    return _positions[column + _centreI];
}
bool
cRowTargets::isAssigned(tSigned32 column) const
{
    assertD(isValidColumn(column));
    return _assigned[column + _centreI] != 0;
}
void
cRowTargets::assign(tSigned32 column, const cCollapsibleGroupAgent* agent)
{
    assertD(isValidColumn(column));
    assertD(_assigned[column + _centreI] == 0);
    _assigned[column + _centreI] = agent;
}
void
cRowTargets::unassign(tSigned32 column, const cCollapsibleGroupAgent* agent)
{
    assertD(isValidColumn(column));
    assertD(_assigned[column + _centreI] == agent);
    _assigned[column + _centreI] = 0;
}
const cCollapsibleGroupAgent*
cRowTargets::assignedAgent(tSigned32 column)
{
    return _assigned[column + _centreI];
}

tSigned32
cRowTargets::closestValidColumn(tSigned32 targetColumn) const
{
    if(targetColumn < firstColumn())
    {
        return firstColumn();
    }
    if(targetColumn > lastColumn())
    {
        return lastColumn();
    }
    return targetColumn;
}

tSigned32
cRowTargets::centringAndWidthAreIdentical(const cRowTargets& rhsTargets) const
{
    return _centreI == rhsTargets._centreI && _positions.size() == rhsTargets._positions.size();
}
