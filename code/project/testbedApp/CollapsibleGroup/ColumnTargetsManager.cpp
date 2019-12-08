#include "base/Header.h"
#include "project/testbedApp/CollapsibleGroup/ColumnTargetsManager.h"
#include "project/testbedApp/CollapsibleGroup/GenerateColumnTargets.h"

cColumnTargetsManager::cColumnTargetsManager(
        const iMesh* mesh,
        iShape* shape,
        const iCollisionContext* context,
        iPath* path,
        tSigned32 agentSpacing, tSigned32 maximumWidth,
        double advanceStep
        )
{
    assertD(path);
    _mesh = mesh;
    _shape = shape;
    _context = context;
    _remainingPath = path;
    _agentSpacing = agentSpacing;
    _maximumWidth = maximumWidth;
    _advanceStep = advanceStep;
    _startIndex = 0;
    _currentSegmentEnd = _remainingPath->position(1);
    _currentSegmentForwardX = _currentSegmentEnd.x - _remainingPath->position(0).x;
    _currentSegmentForwardY = _currentSegmentEnd.y - _remainingPath->position(0).y;
    _atEnd = false;
    _frontRowDistance = 0.;
}
cColumnTargetsManager::~cColumnTargetsManager()
{
    delete _remainingPath;
}

bool
cColumnTargetsManager::atEnd() const
{
    return _atEnd;
}

tSigned32
cColumnTargetsManager::pushFront()
{
    assertD(!_atEnd);

    cPosition basePosition = _remainingPath->position(0);
    std::vector<cPosition> leftTargets, rightTargets;
    GenerateColumnTargets(
            _mesh, _shape, _context,
            basePosition,
            _currentSegmentForwardX, _currentSegmentForwardY,
            _agentSpacing, _maximumWidth,
            leftTargets, rightTargets
            );

    tSigned32 result = _startIndex + SizeL(_targetsWindow);
    _targetsWindow.push_back(cRowTargets(leftTargets, basePosition, rightTargets));

    _frontRowDistance += _advanceStep;
        
    if(_remainingPath->size() == 1)
    {
        _atEnd = true;
    }
    else
    {
        float precisionX, precisionY; // ignored here
        _remainingPath->advanceAlong(_shape, static_cast<float>(_advanceStep), _context, precisionX, precisionY);
        if(_remainingPath->size() > 1 && _remainingPath->position(1) != _currentSegmentEnd)
        {
          // update segment direction vector only on change of path segment
            _currentSegmentEnd = _remainingPath->position(1);
            _currentSegmentForwardX = _currentSegmentEnd.x - _remainingPath->position(0).x;
            _currentSegmentForwardY = _currentSegmentEnd.y - _remainingPath->position(0).y;
        }
    }

    // following added to remove messy looking shorter step distance position set at end of path
    if(_remainingPath->getLength() < _advanceStep)
    {
        _atEnd = true;
    }

    return result;
}
void
cColumnTargetsManager::popBack(tSigned32 rowIndex)
{
    _targetsWindow.pop_front();
    ++_startIndex;
}

cRowTargets&
cColumnTargetsManager::refRow(tSigned32 rowIndex)
{
    assertD(rowIndex >= _startIndex);
    rowIndex -= _startIndex;
    assertD(rowIndex < SizeL(_targetsWindow));
    return _targetsWindow[rowIndex];
}

tSigned32
cColumnTargetsManager::frontRowIndex() const
{
    assertD(!_targetsWindow.empty());
    return _startIndex + SizeL(_targetsWindow) - 1;
}
double
cColumnTargetsManager::frontRowDistance() const
{
    return _frontRowDistance;
}
