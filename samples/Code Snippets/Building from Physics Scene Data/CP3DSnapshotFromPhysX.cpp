
#include "CP3DSnapshotFromPhysX.h"
#include "SolidObjects.h"
#include "AddPhysXConvexShape.h"
#include "PhysXTerrainWrapper.h"
#include "PhysXTriangleMeshWrapper.h"
#include "i_pathengine.h"
#include <NxScene.h> 
#include <NxActor.h> 
#include <NxConvexShape.h> 
#include <NxHeightFieldShape.h> 

void
BuildCP3DSnapshotFromPhysX(
        NxScene* scene,
        std::vector<const iFaceVertexMesh*>& faceVertexMeshes, 
        iSolidObjects*& solidObjectsResult,
        const cCoordinateMapping& coordinateMapping,
        float scale,
        bool includeDynamicComponents
        )
{
    tSigned32 numberOfActors = scene->getNbActors();
    NxActor** actors = scene->getActors();
    cSolidObjects* solidObjects = new cSolidObjects;
    solidObjectsResult = solidObjects;
    for(tSigned32 i = 0; i != numberOfActors; ++i)
    {
        NxActor& actor = *actors[i];
        if(!includeDynamicComponents && actor.isDynamic())
        {
            continue;
        }
        tSigned32 numberOfShapes = actor.getNbShapes();
        NxShape *const* shapes = actor.getShapes(); 
        for(tSigned32 j = 0; j != numberOfShapes; ++j)
        {
            NxShape& shape = *shapes[j];
            bool wasProcessedAsConvex = AddPhysXConvexShape(shape, scale, *solidObjects);
            if(wasProcessedAsConvex)
            {
                continue;
            }
            NxShapeType type = shape.getType();
            if(type == NX_SHAPE_MESH)
            {
                NxTriangleMeshShape* concreteShape = shape.isTriangleMesh();
                faceVertexMeshes.push_back(new cPhysXTriangleMeshWrapper(*concreteShape, coordinateMapping, scale));
            }
            else if(type == NX_SHAPE_HEIGHTFIELD)
            {
                NxHeightFieldShape* concreteShape = shape.isHeightField();
                faceVertexMeshes.push_back(new cPhysXTerrainWrapper(*concreteShape, coordinateMapping, scale));
            }
            else
            {
                //..... TODO - add wrappers / solid objects support for any other types that are required
            }
        }
    }
    solidObjects->applyCoordinateMapping(coordinateMapping);
}

void
DeleteCP3DSnapshotFromPhysX(
        std::vector<const iFaceVertexMesh*>& faceVertexMeshes, 
        iSolidObjects*& solidObjectsResult
        )
{
    for(tSigned32 i = 0; i != static_cast<tSigned32>(faceVertexMeshes.size()); ++i)
    {
        delete faceVertexMeshes[i];
    }
    delete solidObjectsResult;
}
