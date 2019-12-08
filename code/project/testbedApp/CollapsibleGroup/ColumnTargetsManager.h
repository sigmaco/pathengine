#ifndef COLUMN_TARGETS_MANAGER_INCLUDED
#define COLUMN_TARGETS_MANAGER_INCLUDED

#include "project/testbedApp/CollapsibleGroup/RowTargets.h"
#include <vector>
#include <deque>

class cColumnTargetsManager
{
    const iMesh* _mesh;
    iShape* _shape;
    const iCollisionContext* _context;
    iPath* _remainingPath;
    tSigned32 _agentSpacing, _maximumWidth;
    double _advanceStep;
    cPosition _currentSegmentEnd;
    tSigned32 _currentSegmentForwardX, _currentSegmentForwardY;
    std::deque<cRowTargets> _targetsWindow;
    tSigned32 _startIndex;
    bool _atEnd;
    double _frontRowDistance;

    // prevent copying and assignment
    cColumnTargetsManager(const cColumnTargetsManager&);
    const cColumnTargetsManager& operator=(const cColumnTargetsManager&);

public:

    // mesh, shape and context pointers must remain valid over lifetime of the constructed object
    // takes ownership of the path object
    cColumnTargetsManager(
            const iMesh* mesh,
            iShape* shape,
            const iCollisionContext* context,
            iPath* path,
            tSigned32 agentSpacing, tSigned32 maximumWidth,
            double advanceStep
            );
    ~cColumnTargetsManager();

    bool atEnd() const;
    tSigned32 pushFront(); // returns index of added row
    void popBack(tSigned32 rowIndex);
    cRowTargets& refRow(tSigned32 rowIndex);

    tSigned32 frontRowIndex() const;
    double frontRowDistance() const;
};

#endif
