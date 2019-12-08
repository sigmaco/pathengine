
#include "ExampleBuildFromHavok.h"
#include "CoordinateMapping.h"
#include "CP3DSnapshotFromHavok.h"
#include "i_pathengine.h"
#include "i_contentprocessing3d.h"
#include <vector>
#include <stdio.h> 
#include <assert.h> 

class cFileOutputStream : public iOutputStream
{
    FILE* _file;
public:
    cFileOutputStream(const char* name)
    {
        _file = fopen(name, "wb");
        assert(_file);
    }
    ~cFileOutputStream()
    {
        int errorOccurred = fclose(_file);
        assert(!errorOccurred);
    }
    void put(const char* data, unsigned long dataSize)
    {
        size_t written = fwrite(data, 1, dataSize, _file);
        assert(written == dataSize);
        int flushResult = fflush(_file);
        assert(flushResult == 0);
    }
};

void
ExampleBuildFromHavok(hkpWorld* world, iContentProcessing3D* cp3d)
{
    const float buildScale = 100.f; // recommended Havok scale is 1 metre units, for PathEngine smaller units are generally required (e.g., in this case, 1cm)
    cCoordinateMapping coordinateMapping;
    // ** no coordinate mapping was required when running against the Havok demos,
    // ** but depending on the coordinate system actually being used by the application,
    // ** something like the following may be required:
    //coordinateMapping.swap(1, 2);     // swap y and z values
    //coordinateMapping.negate(0);      // negate x value (applied after any swaps)

    iSolidObjects* solidObjects;
    std::vector<const iFaceVertexMesh*> faceVertexMeshes;
    BuildCP3DSnapshotFromHavok(world, faceVertexMeshes, solidObjects, coordinateMapping, buildScale);


  // 3D processing parameters and options setup
  // (adjust according to actual scene and agent scale!)
    const tSigned32 extrudeHeight = 280;
    const tSigned32 maxStepHeight = 80;
    const tSigned32 maxSlope = 90;
    const char* voxelOptions[] = {
            "method", "voxels",
            "voxelSize", "100",
            "optimiseWithThreshold", "150",
            //"stripTerrainHeightDetail", "true",
            //"subdivisionSize", "4",
            "excludeDownwardFacingFromGroundResult", "true",
            0
    };
    const char* bspOptions[] = {
            "method", "bsp",
            //"removeObstructedFragmentsWithShape", "80,200,200,80,200,-80,80,-200,-80,-200,-200,-80,-200,80,-80,200",
            //"excludeDownwardFacingFromGroundResult", "true",
            0
    };
    const bool useVoxels = false;

    {
        cFileOutputStream fos("snapshot.tok");
        cp3d->saveSnapshot(
                faceVertexMeshes.empty() ? 0 : &faceVertexMeshes[0],
                static_cast<tSigned32>(faceVertexMeshes.size()),
                solidObjects,
                extrudeHeight, maxStepHeight, maxSlope,
                useVoxels ? voxelOptions : bspOptions,
                "tok", &fos
                );
    }
    {
        iMesh* mesh = cp3d->buildGround(
                faceVertexMeshes.empty() ? 0 : &faceVertexMeshes[0],
                static_cast<tSigned32>(faceVertexMeshes.size()),
                solidObjects,
                extrudeHeight, maxStepHeight, maxSlope,
                useVoxels ? voxelOptions : bspOptions,
                0
                );
        if(mesh)
        {
            cFileOutputStream fos("mesh.tok");
            mesh->saveGround("tok", false, &fos);
        }
    }

    DeleteCP3DSnapshotFromHavok(faceVertexMeshes, solidObjects);
}
