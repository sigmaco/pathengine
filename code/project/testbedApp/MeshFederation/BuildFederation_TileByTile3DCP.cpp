
#include "base/Header.h"
#include "project/testbedApp/MeshFederation/BuildFederation_TileByTile3DCP.h"
#include "sampleShared/SimpleDOM.h"
#include "sampleShared/LoadBinary.h"
#include "common/FileOutputStream.h"
#include "common/STL_Helper.h"
#include "externalAPI/i_pathengine.h"
#include "externalAPI/i_testbed.h"
#include <string>
#include <fstream>
#include <sstream>

static void
BuildTile(
        iTestBed* testBed,
        iMeshFederation* federation,
        tSigned32 tileIndex,
        const iSourceTiling& tiling,
        const std::vector<const iMesh*>& sourceTiles
        )
{
    cVectorReceiver<tSigned32> tileMeshIndicesReceiver;
    federation->getGroundTilesOverlapped(tileIndex, &tiling, tileMeshIndicesReceiver);
    const std::vector<tSigned32>& tileMeshIndices = tileMeshIndicesReceiver.vector;
    tSigned32 numberOfTileMeshes = tileMeshIndices.size();
    std::vector<const iMesh*> tileMeshes(numberOfTileMeshes);
    tSigned32 i;
    for(i = 0; i != numberOfTileMeshes; ++i)
    {
        tileMeshes[i] = sourceTiles[tileMeshIndices[i]];
    }
    iMesh* tileMesh = federation->buildTileMeshFromLocalGroundTiles(
            tileIndex, &tiling,
            &tileMeshes[0], numberOfTileMeshes,
            0
            );
    std::ostringstream fileName;
    fileName << "federationTile" << tileIndex << ".tok";
    cFileOutputStream fos(fileName.str().c_str());
    if(tileMesh)
    {
        tileMesh->saveGround("tok", true, fos);
        delete tileMesh;
    }
    testBed->printTextLine(5, fileName.str().c_str());
    testBed->printTextLine(5, "finished generating federation tile:");
    testBed->printTextLine(5, "Generating Federation Tile Meshes");
    testBed->update_ExitIfRequested();
}

void
LoadAllSourceTiles(
        iPathEngine* pathEngine,
        tSigned32 numberToLoad,
        std::vector<const iMesh*>& result
        )
{
    tSigned32 i;
    for(i = 0; i != numberToLoad; ++i)
    {
        std::string meshName;
        {
            std::ostringstream oss;
            oss << "../tileResult" << i << ".tok";
            meshName = oss.str();
        }
        std::vector<char> meshBuffer;
        LoadBinary(meshName.c_str(), meshBuffer);
        if(meshBuffer.empty())
        {
            result.push_back(0);
        }
        else
        {
            iMesh* sourceTile = pathEngine->loadMeshFromBuffer(
                    "tok",
                    VectorBuffer(meshBuffer), SizeL(meshBuffer),
                    0
                    );
            result.push_back(sourceTile);
        }
    }
}

void
BuildFederation_TileByTile3DCP(
        iPathEngine* pathEngine, iTestBed* testBed,
        tSigned32 tileSize, tSigned32 overlap
        )
{
    iSourceTiling* tiling;
    if(!FileExists("tiling.tok"))
    {
        Error("Fatal", "Tiling file not found. Please run the 3D content processing first to generated the necessary tiling and tiled ground result pieces.");
    }
    {
        std::vector<char> tilingBuffer;
        LoadBinary("tiling.tok", tilingBuffer);
        tiling = pathEngine->loadSourceTiling("tok", VectorBuffer(tilingBuffer), SizeL(tilingBuffer));
    }

    std::vector<const iMesh*> sourceTiles;
    LoadAllSourceTiles(pathEngine, tiling->size(), sourceTiles);

    cHorizontalRange worldRange;
    tiling->getWorldRange(worldRange);

    iMeshFederation* federation = pathEngine->buildMeshFederation_TilingOnly(worldRange, tileSize, overlap);

    tSigned32 i;
    for(i = 0; i != federation->size(); ++i)
    {
        BuildTile(testBed, federation, i, *tiling, sourceTiles);
    }
    delete tiling;

    std::ostringstream fileName;
    fileName << "federation.tok";
    cFileOutputStream fos(fileName.str().c_str());
    federation->save("tok", fos);
    delete federation;
}

