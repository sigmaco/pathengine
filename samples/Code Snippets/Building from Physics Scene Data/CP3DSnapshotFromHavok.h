
#include <vector>

class hkpWorld;
class iFaceVertexMesh;
class iSolidObjects;
class cCoordinateMapping;

void
BuildCP3DSnapshotFromHavok(
        hkpWorld* world,
        std::vector<const iFaceVertexMesh*>& faceVertexMeshes, 
        iSolidObjects*& solidObjectsResult,
        const cCoordinateMapping& coordinateMapping,
        float scale = 1.f
        );

void
DeleteCP3DSnapshotFromHavok(
        std::vector<const iFaceVertexMesh*>& faceVertexMeshes, 
        iSolidObjects*& solidObjectsResult
        );

