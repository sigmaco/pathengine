
#include "base/Header.h"
#include "project/testbedApp/MeshFederation/BuildFederation_FromWorldMesh.h"
#include "common/FileOutputStream.h"
#include "externalAPI/i_pathengine.h"
#include "externalAPI/i_testbed.h"
#include <sstream>

namespace
{

class cSaveTileMeshesCallBack : public iMeshFederationTileGeneratedCallBack
{
    iTestBed* _testBed;

public:

    cSaveTileMeshesCallBack(iTestBed* testBed);
    void tileGenerated(tSigned32 tileIndex, iMesh* tileMesh);
};

} // end anonymous namespace

cSaveTileMeshesCallBack::cSaveTileMeshesCallBack(iTestBed* testBed) :
 _testBed(testBed)
{
}

void
cSaveTileMeshesCallBack::tileGenerated(tSigned32 tileIndex, iMesh* tileMesh)
{
    std::ostringstream fileName;
    fileName << "federationTile" << tileIndex << ".tok";
    cFileOutputStream fos(fileName.str().c_str());
    tileMesh->saveGround("tok", true, fos);
    delete tileMesh;
    _testBed->printTextLine(5, fileName.str().c_str());
    _testBed->printTextLine(5, "finished generating federation tile:");
    _testBed->printTextLine(5, "Generating Federation Tile Meshes");
    _testBed->update_ExitIfRequested();
}

void
BuildFederation(
        iPathEngine* pathEngine, iTestBed* testBed,
        iMesh* worldMesh,
        tSigned32 tileSize, tSigned32 overlap
        )
{
    cSaveTileMeshesCallBack callback(testBed);

    iMeshFederation* federation;
    federation = pathEngine->buildMeshFederation_FromWorldMesh(
            worldMesh,
            tileSize, overlap,
            callback
            );

    std::ostringstream fileName;
    fileName << "federation.tok";
    cFileOutputStream fos(fileName.str().c_str());
    federation->save("tok", fos);
    delete federation;
}

void
BuildFederation_FromWorldMesh(
        iPathEngine* pathEngine, iTestBed* testBed,
        const char* worldMeshName,
        tSigned32 tileSize, tSigned32 overlap
        )
{
    iMesh* worldMesh;
    {
        char* buffer;
        tUnsigned32 size;
        std::string meshPath = "../resource/meshes/";
        meshPath.append(worldMeshName);
        meshPath.append(".tok");
        buffer = testBed->loadBinary(meshPath.c_str(), size);
        worldMesh = pathEngine->loadMeshFromBuffer("tok", buffer, size, 0);
        testBed->freeBuffer(buffer);
    }
    BuildFederation(pathEngine, testBed, worldMesh, tileSize, overlap);
}
