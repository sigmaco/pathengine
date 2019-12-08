
#include "CP3DSnapshotFromHavok.h"
#include "AddHavokConvexShape.h"
#include "SolidObjects.h"
#include "FaceVertexMesh.h"
#include "i_pathengine.h"
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/World/hkpSimulationIsland.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Collide/Shape/Convex/Triangle/hkpTriangleShape.h>

static void
AddTriangle(const hkpTriangleShape& shape, const hkTransform& transform, float scale, cFaceVertexMesh& addTo)
{
    tSigned32 vertexCoords[9];
    for(int i = 0; i != 3; ++i)
    {
        // note that winding order is reversed here, based on the scene tested against (the scene CharacterControlVsDemo)
        hkVector4 v = shape.getVertex(2 - i);
        hkVector4 transformedV;
        transformedV.setTransformedPos(transform, v);
        transformedV.mul4(scale);
        for(int j = 0; j != 3; ++j)
        {
            vertexCoords[i * 3 + j] = static_cast<tSigned32>(transformedV(j));
        }
    }
    addTo.addTri(vertexCoords);
}

static void
AddShape(const hkpShape& shape, const hkTransform& transform, float scale, cSolidObjects& solidObjects, cFaceVertexMesh& faceVertexMesh)
{
    hkpShapeType shapeType = shape.getType();

    if(shapeType == HK_SHAPE_TRIANGLE)
    {
        AddTriangle(static_cast<const hkpTriangleShape&>(shape), transform, scale, faceVertexMesh);
        return;
    }

    if(shape.isConvex())
    {
        AddHavokConvexShape(static_cast<const hkpConvexShape&>(shape), transform, scale, solidObjects);
        return;
    }

    const hkpShapeContainer* container = shape.getContainer();

    if(container)
    {
        hkpShapeContainer::ShapeBuffer shapeBuffer;
        int numChildren = container->getNumChildShapes();
        hkpShapeKey key = container->getFirstKey();
        while(key != HK_INVALID_SHAPE_KEY)
        {
            const hkpShape* childShape = container->getChildShape(key, shapeBuffer);
            hkpShapeType childType = childShape->getType();
            AddShape(*childShape, transform, scale, solidObjects, faceVertexMesh);
            key = container->getNextKey(key);
        }
    }
}

void
BuildCP3DSnapshotFromHavok(
        hkpWorld* world,
        std::vector<const iFaceVertexMesh*>& faceVertexMeshes, 
        iSolidObjects*& solidObjectsResult,
        const cCoordinateMapping& coordinateMapping,
        float scale
        )
{
    const hkpSimulationIsland* island = world->getFixedIsland();
    const hkArray<hkpEntity *>& entities = island->getEntities();
    cSolidObjects* solidObjects = new cSolidObjects;
    solidObjectsResult = solidObjects;
    cFaceVertexMesh* faceVertexMesh = new cFaceVertexMesh;
    faceVertexMeshes.push_back(faceVertexMesh);
    long i;
    for(i = 0; i != entities.getSize(); ++i)
    {
        hkpEntity& entity = *entities[i];
        //if(!entity.isFixed())
        //{
        //    continue;
        //}
        const hkpCollidable* collidable = entity.getCollidable();
        if(collidable == 0)
        {
            continue;
        }
        const hkpShape* shape = collidable->getShape();
        if(shape == 0)
        {
            continue;
        }
        AddShape(*shape, collidable->getTransform(), scale, *solidObjects, *faceVertexMesh);
    }
    solidObjects->applyCoordinateMapping(coordinateMapping);
    faceVertexMesh->applyCoordinateMapping(coordinateMapping);
}

void
DeleteCP3DSnapshotFromHavok(
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
