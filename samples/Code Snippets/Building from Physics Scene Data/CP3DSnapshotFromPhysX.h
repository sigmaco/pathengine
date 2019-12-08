
#include <vector>

class NxScene;
class iFaceVertexMesh;
class iSolidObjects;
class cCoordinateMapping;

void
BuildCP3DSnapshotFromPhysX(
        NxScene* scene,
        std::vector<const iFaceVertexMesh*>& faceVertexMeshes, 
        iSolidObjects*& solidObjectsResult,
        const cCoordinateMapping& coordinateMapping,
        float scale = 1.f,
        bool includeDynamicComponents = false
        );

void
DeleteCP3DSnapshotFromPhysX(
        std::vector<const iFaceVertexMesh*>& faceVertexMeshes, 
        iSolidObjects*& solidObjectsResult
        );

