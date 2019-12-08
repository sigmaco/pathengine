//**********************************************************************
//
// Copyright (c) 2002-2014
// Thomas Young, PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef PATHENGINE_INTERFACE_HAS_BEEN_INCLUDED
#define PATHENGINE_INTERFACE_HAS_BEEN_INCLUDED

#include <vector>

#if defined(__GNUC__) && defined(__x86_64__)
typedef int tSigned32;
typedef unsigned int tUnsigned32;
typedef long tSigned64;
#else
typedef long tSigned32;
typedef unsigned long tUnsigned32;
#if defined(__GNUC__)
typedef long long tSigned64;
#else
typedef __int64 tSigned64;
#endif
#endif

const tSigned32 PATHENGINE_INTERFACE_MAJOR_VERSION = 18;
const tSigned32 PATHENGINE_INTERFACE_MINOR_VERSION = 0;

#ifndef PATHENGINE_POD_CLASSES_HAVE_BEEN_DEFINED
#define PATHENGINE_POD_CLASSES_HAVE_BEEN_DEFINED
class cPosition
{
public:
    cPosition() : cell(-1) {}
    tSigned32 x, y, cell;
    bool operator==(const cPosition& rhs) const
    {
        if(cell == -1)
            return cell == rhs.cell;
        return cell == rhs.cell && x == rhs.x && y == rhs.y;
    }
    bool operator!=(const cPosition& rhs) const
    {
        return !((*this) == rhs);
    }
};
class cHorizontalRange
{
public:
    tSigned32 minX, minY, maxX, maxY;
    bool operator==(const cHorizontalRange& rhs) const
    {
        return minX == rhs.minX && minY == rhs.minY && maxX == rhs.maxX && maxY == rhs.maxY;
    }
    bool operator!=(const cHorizontalRange& rhs) const
    {
        return !((*this) == rhs);
    }
};
#endif

#ifndef PATHENGINE_iArrayReceiver_DEFINED
#define PATHENGINE_iArrayReceiver_DEFINED
template <class T> class iArrayReceiver
{
public:
    virtual ~iArrayReceiver() {}
    virtual void receive(const T& element) = 0;
};
#endif
#ifndef PATHENGINE_cVectorReceiver_DEFINED
#define PATHENGINE_cVectorReceiver_DEFINED
template <class T> class cVectorReceiver : public iArrayReceiver<T>
{
public:
    std::vector<T> vector;
    void
    receive(const T& element)
    {
        vector.push_back(element);
    }
    T*
    buffer()
    {
        return vector.empty() ? 0 : &vector.front();
    }
    tUnsigned32
    bufferElements() const
    {
        return static_cast<tUnsigned32>(vector.size());
    }
};
#endif

class iFaceVertexMesh;
class iOutputStream;
class iAgent;
class iMesh;
class iPathEngine;
class iShape;
class iCollisionContext;
class iObstacleSet;
class iPath;
class iCollisionInfo;
class iTerrainCallBack;
class iRender3DLinesCallBack;
class iQueryCallBack;
class iContentChunk;
class iContentChunkInstance;
class iMeshFederation;
class iSourceTiling;
class iAnchorsAndPinnedShapes;
class iSolidObjects;
class iMeshFederationTileGeneratedCallBack;
class iConnectionTestCallBack;
class iProgressCallBack;
class iErrorHandler;

#ifndef PATHENGINE_iFaceVertexMesh_DEFINED
#define PATHENGINE_iFaceVertexMesh_DEFINED
class iFaceVertexMesh
{
public:
    virtual ~iFaceVertexMesh() {}
    virtual tSigned32 faces() const = 0;
    virtual tSigned32 vertices() const = 0;
    virtual tSigned32 vertexIndex(tSigned32 face, tSigned32 vertexInFace) const = 0;
    virtual tSigned32 vertexX(tSigned32 vertexIndex) const = 0;
    virtual tSigned32 vertexY(tSigned32 vertexIndex) const = 0;
    virtual float vertexZ(tSigned32 vertexIndex) const = 0;
    virtual tSigned32 faceAttribute(tSigned32 face, tSigned32 attributeIndex) const = 0;
};
#endif

#ifndef PATHENGINE_iOutputStream_DEFINED
#define PATHENGINE_iOutputStream_DEFINED
class iOutputStream
{
public:
    virtual ~iOutputStream() {}
    virtual void put(char const * data_Buffer, tUnsigned32 data_BufferEntries) = 0;
};
#endif

#ifndef PATHENGINE_iAgent_DEFINED
#define PATHENGINE_iAgent_DEFINED
class iAgent
{
public:
    virtual void destroy() = 0;
    virtual iMesh* getMesh() const = 0;
    virtual const iShape* getShape() const = 0;
    virtual void setUserData(tSigned64 value) = 0;
    virtual tSigned64 getUserData() const = 0;
    virtual cPosition getPosition() const = 0;
    virtual void moveTo(const cPosition& newPosition) = 0;
    virtual bool canCollide() const = 0;
    virtual bool canPathfind(const iCollisionContext* context) const = 0;
    virtual bool testCollisionAt(const iCollisionContext* context, const cPosition& newPosition) const = 0;
    virtual bool testCollisionTo(const iCollisionContext* context, const cPosition& newPosition) const = 0;
    virtual bool testCollisionTo_XY(const iCollisionContext* context, tSigned32 x, tSigned32 y, tSigned32& cell) const = 0;
    virtual iCollisionInfo* firstCollisionTo(const iCollisionContext* context, tSigned32 x, tSigned32 y, tSigned32& cell) const = 0;
    virtual cPosition findClosestUnobstructedPosition(const iCollisionContext* context, tSigned32 maximumDistance) const = 0;
    virtual void getAllAgentsOverlapped(const iCollisionContext* context, iArrayReceiver<iAgent*>& overlappedAgents) const = 0;
    virtual bool testCollisionDirectlyAgainstPlacedShape(const iShape* shape, const cPosition& shapePosition) const = 0;
    virtual iPath* findShortestPathTo(const iCollisionContext* context, const cPosition& target) const = 0;
    virtual iPath* findShortestPathTo_WithQueryCallBack(const iCollisionContext* context, const cPosition& target, iQueryCallBack* queryCallBack) const = 0;
    virtual iPath* findPathAway(const iCollisionContext* context, const cPosition& awayFrom, tSigned32 distanceAway) const = 0;
    virtual iPath* findPathAway_WithQueryCallBack(const iCollisionContext* context, const cPosition& awayFrom, tSigned32 distanceAway, iQueryCallBack* queryCallBack) const = 0;
    virtual iCollisionInfo* advanceAlongPath(iPath* path, float distance, const iCollisionContext* context) = 0;
    virtual iCollisionInfo* advanceAlongPathWithPrecision(iPath* path, float distance, const iCollisionContext* context, float& precisionX, float& precisionY) = 0;
    virtual iPath* generateCurvedPath(iPath* basePath, const iCollisionContext* context, tSigned32 startVectorX, tSigned32 startVectorY, tSigned32 sectionLength, float turnRatio1, float turnRatio2) const = 0;
    virtual void setTraverseCost(float cost) = 0;
    virtual void setTraverseCostDirection(tSigned32 directionVectorX, tSigned32 directionVectorY) = 0;
    virtual float getTraverseCost() const = 0;
    virtual void getTraverseCostDirection(tSigned32& directionVectorX, tSigned32& directionVectorY) const = 0;
    virtual iPath* generateCurvedPath_WithEndVector(iPath* basePath, const iCollisionContext* context, tSigned32 startVectorX, tSigned32 startVectorY, tSigned32 endVectorX, tSigned32 endVectorY, tSigned32 sectionLength, float turnRatio1, float turnRatio2) const = 0;
    virtual bool isMoveable() const = 0;
    virtual tSigned32 getNumberOfVertices() const = 0;
    virtual void getVertexCoordinates(tSigned32 index, tSigned32& x, tSigned32& y) const = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iAgent* afterCast = static_cast<iAgent*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iMesh_DEFINED
#define PATHENGINE_iMesh_DEFINED
class iMesh
{
public:
    virtual void destroy() = 0;
    virtual tSigned32 getNumberOf3DFaces() const = 0;
    virtual tSigned32 get3DFaceAtPosition(const cPosition& position) const = 0;
    virtual void get3DFaceVertex(tSigned32 faceIndex, tSigned32 vertexIndexInFace, tSigned32& x, tSigned32& y, float& z) const = 0;
    virtual tSigned32 get3DFaceConnection(tSigned32 faceIndex, tSigned32 edgeIndexInFace) const = 0;
    virtual void get3DFaceNormal(tSigned32 faceIndex, float& resultX, float& resultY, float& resultZ) const = 0;
    virtual tSigned32 get3DFaceAttribute(tSigned32 faceIndex, tSigned32 attributeIndex) const = 0;
    virtual tSigned32 getNumberOfSections() const = 0;
    virtual tSigned32 getSurfaceType(const cPosition& position) const = 0;
    virtual tSigned32 getSectionID(const cPosition& position) const = 0;
    virtual void storeAnchor(const char* id, const cPosition& position, tSigned32 orientation) = 0;
    virtual cPosition retrieveAnchor(const char* id, tSigned32& orientation) const = 0;
    virtual tSigned32 getNumberOfAnchors() const = 0;
    virtual void retrieveAnchorByIndex(tSigned32 index, cPosition& position, tSigned32& orientation, const char*& id) const = 0;
    virtual void storeNamedObstacle(const char* id, iAgent* agent) = 0;
    virtual iAgent* retrieveNamedObstacle(const char* id) const = 0;
    virtual tSigned32 getNumberOfNamedObstacles() const = 0;
    virtual void retrieveNamedObstacleByIndex(tSigned32 index, iAgent*& agent, const char*& id) const = 0;
    virtual cPosition positionFor3DPoint(tSigned32 x, tSigned32 y, tSigned32 z) const = 0;
    virtual cPosition positionNear3DPoint(tSigned32 x, tSigned32 y, tSigned32 z, tSigned32 horizontalRange, tSigned32 verticalRange) const = 0;
    virtual cPosition positionInSection(tSigned32 sectionID, tSigned32 x, tSigned32 y) const = 0;
    virtual cPosition positionInSectionFor3DPoint(tSigned32 sectionID, tSigned32 x, tSigned32 y, tSigned32 z) const = 0;
    virtual cPosition positionInSectionNear3DPoint(tSigned32 sectionID, tSigned32 x, tSigned32 y, tSigned32 z, tSigned32 horizontalRange, tSigned32 verticalRange) const = 0;
    virtual void resolvePositionsNear3DPoint(tSigned32 sectionID, tSigned32 x, tSigned32 y, tSigned32 z, const cHorizontalRange& horizontalQueryRegion, tSigned32 minimumZ, tSigned32 maximumZ, iArrayReceiver<cPosition>& positions) const = 0;
    virtual cPosition positionFor3DPoint_ExcludeTerrain(tSigned32 x, tSigned32 y, tSigned32 z, tSigned32 numberOfTerrainLayers) const = 0;
    virtual cPosition generateRandomPosition() const = 0;
    virtual cPosition generateRandomPositionLocally(const cPosition& centre, tSigned32 range) const = 0;
    virtual cPosition generateRandomPositionInSection(tSigned32 sectionID) const = 0;
    virtual tSigned32 heightAtPosition(const cPosition& position) const = 0;
    virtual float heightAtPositionF(const cPosition& position) const = 0;
    virtual float heightAtPositionWithPrecision(const cPosition& position, float precisionX, float precisionY) const = 0;
    virtual tSigned32 getCellForEndOfLine(const cPosition& start, tSigned32 endX, tSigned32 endY) const = 0;
    virtual bool positionIsValid(const cPosition& p) const = 0;
    virtual void burnContextIntoMesh(const iCollisionContext* context) = 0;
    virtual void saveGround(const char* format, bool includeMapping, iOutputStream& outputStream) const = 0;
    virtual void setTerrainCallBack(tSigned32 terrainLayer, iTerrainCallBack* callBack) = 0;
    virtual tSigned32 addEndPoint(const cPosition& position) = 0;
    virtual tSigned32 addOffMeshConnection(tSigned32 fromEndPoint, tSigned32 toEndPoint, tSigned32 penalty) = 0;
    virtual tSigned32 getNumberOfEndPoints() const = 0;
    virtual cPosition getEndPoint(tSigned32 index) const = 0;
    virtual tSigned32 getNumberOfOffMeshConnections() const = 0;
    virtual void getOffMeshConnectionInfo(tSigned32 index, tSigned32& fromEndPoint, tSigned32& toEndPoint, tSigned32& penalty) const = 0;
    virtual void clearOffMeshConnectionsAndEndPoints() = 0;
    virtual void generateUnobstructedSpaceFor(const iShape* shape, bool combine, const char *const* options) = 0;
    virtual void releaseUnobstructedSpaceFor(const iShape* shape) = 0;
    virtual void generateConnectedRegionPreprocessFor(const iShape* shape, const char *const* options) = 0;
    virtual void releaseConnectedRegionPreprocessFor(const iShape* shape) = 0;
    virtual void generatePathfindPreprocessFor(const iShape* shape, const char *const* options) = 0;
    virtual void releasePathfindPreprocessFor(const iShape* shape) = 0;
    virtual void preprocessGenerationCompleted() = 0;
    virtual bool shapeCanCollide(const iShape* shape) const = 0;
    virtual bool shapeHasCombinedUnobstructedSpace(const iShape* shape) const = 0;
    virtual bool shapeHasConnectedRegionPreprocess(const iShape* shape) const = 0;
    virtual bool shapeCanPathfind(const iShape* shape, const iCollisionContext* context) const = 0;
    virtual void saveUnobstructedSpaceFor(const iShape* shape, iOutputStream& os) const = 0;
    virtual void loadUnobstructedSpaceFor(const iShape* shape, char const * data_Buffer, tUnsigned32 data_BufferEntries) = 0;
    virtual void saveConnectedRegionPreprocessFor(const iShape* shape, iOutputStream& os) const = 0;
    virtual void loadConnectedRegionPreprocessFor(const iShape* shape, char const * data_Buffer, tUnsigned32 data_BufferEntries) = 0;
    virtual void savePathfindPreprocessFor(const iShape* shape, iOutputStream& os) const = 0;
    virtual void loadPathfindPreprocessFor(const iShape* shape, char const * data_Buffer, tUnsigned32 data_BufferEntries) = 0;
    virtual iAgent* placeAgent(const iShape* shape, const cPosition& position) const = 0;
    virtual iCollisionContext* newContext() const = 0;
    virtual iObstacleSet* newObstacleSet() const = 0;
    virtual bool testPointCollision(const iShape* shape, const iCollisionContext* context, const cPosition& position) const = 0;
    virtual bool testLineCollision(const iShape* shape, const iCollisionContext* context, const cPosition& start, const cPosition& end) const = 0;
    virtual bool testLineCollision_XY(const iShape* shape, const iCollisionContext* context, const cPosition& start, tSigned32 x, tSigned32 y, tSigned32& cell) const = 0;
    virtual iCollisionInfo* firstCollision(const iShape* shape, const iCollisionContext* context, const cPosition& start, tSigned32 x, tSigned32 y, tSigned32& cell) const = 0;
    virtual cPosition findClosestUnobstructedPosition(const iShape* shape, const iCollisionContext* context, const cPosition& position, tSigned32 maximumDistance) const = 0;
    virtual cPosition findClosestUnobstructedPositionEx(const iShape* shape, const iCollisionContext* context, cPosition const * rootPositions_Buffer, tUnsigned32 rootPositions_BufferEntries, const cHorizontalRange& queryRegion, tSigned32 targetX, tSigned32 targetY, tSigned32 const * permittedConnectedRegions_Buffer, tUnsigned32 permittedConnectedRegions_BufferEntries) const = 0;
    virtual void getAllAgentsOverlapped(const iShape* shape, const iCollisionContext* context, const cPosition& position, iArrayReceiver<iAgent*>& overlappedAgents) const = 0;
    virtual iPath* findShortestPath(const iShape* shape, const iCollisionContext* context, const cPosition& start, const cPosition& goal) const = 0;
    virtual iPath* findShortestPath_WithQueryCallBack(const iShape* shape, const iCollisionContext* context, const cPosition& start, const cPosition& goal, iQueryCallBack* queryCallBack) const = 0;
    virtual iPath* findPathAway(const iShape* shape, const iCollisionContext* context, const cPosition& start, const cPosition& awayFrom, tSigned32 distanceAway) const = 0;
    virtual iPath* findPathAway_WithQueryCallBack(const iShape* shape, const iCollisionContext* context, const cPosition& start, const cPosition& awayFrom, tSigned32 distanceAway, iQueryCallBack* queryCallBack) const = 0;
    virtual iPath* generateCurvedPath(const iShape* shape, iPath* basePath, const iCollisionContext* context, tSigned32 startVectorX, tSigned32 startVectorY, tSigned32 sectionLength, float turnRatio1, float turnRatio2) const = 0;
    virtual iPath* constructPath(cPosition const * positions_Buffer, tUnsigned32 positions_BufferEntries, tSigned32 const * connectionIndices_Buffer, tUnsigned32 connectionIndices_BufferEntries) const = 0;
    virtual iPath* constructPath_Reversed(cPosition const * positions_Buffer, tUnsigned32 positions_BufferEntries, tSigned32 const * connectionIndices_Buffer, tUnsigned32 connectionIndices_BufferEntries) const = 0;
    virtual void savePath(iPath* path, iOutputStream& os) const = 0;
    virtual iPath* loadPath(char const * data_Buffer, tUnsigned32 data_BufferEntries) const = 0;
    virtual void renderLineOnGround(const cPosition& start, tSigned32 endX, tSigned32 endY, tSigned32 originX, tSigned32 originY, iRender3DLinesCallBack& callBack) const = 0;
    virtual iObstacleSet* newObstacleSet_WithAttributes(const char *const* attributes) const = 0;
    virtual cPosition positionInSectionInShape(tSigned32 sectionID, tSigned32 shapeOriginX, tSigned32 shapeOriginY, tSigned32 const * shapeCoordinates_Buffer, tUnsigned32 shapeCoordinates_BufferEntries) const = 0;
    virtual iPath* generateCurvedPath_WithEndVector(const iShape* shape, iPath* basePath, const iCollisionContext* context, tSigned32 startVectorX, tSigned32 startVectorY, tSigned32 endVectorX, tSigned32 endVectorY, tSigned32 sectionLength, float turnRatio1, float turnRatio2) const = 0;
    virtual iAgent* placeLargeStaticObstacle(tSigned32 const * shapeCoordinates_Buffer, tUnsigned32 shapeCoordinates_BufferEntries, const cPosition& position) const = 0;
    virtual iAgent* placeProjected3DObstruction(const iFaceVertexMesh* facesToProject, const cPosition& rootFrom, tSigned32 agentHeight) const = 0;
    virtual void addAnchorsAndShapes(const iAnchorsAndPinnedShapes* anchorsAndShapes, const char* idPrefix, tSigned32 numberOfTerrainLayers) = 0;
    virtual void autoGenerateConnections(tSigned32 sampleSpacing, tSigned32 localityConstraint, tSigned32 horizontalRange, tSigned32 verticalRange, tSigned32 dropRange, iConnectionTestCallBack& callBack) = 0;
    virtual tSigned32 getNumberOfBurntInObstacles() const = 0;
    virtual cPosition getBurntInObstacleRoot(tSigned32 index) const = 0;
    virtual tSigned32 getBurntInObstacleVertices(tSigned32 index) const = 0;
    virtual void getBurntInObstacleVertex(tSigned32 obstacleIndex, tSigned32 vertexIndex, tSigned32& x, tSigned32& y) const = 0;
    virtual tSigned32 getNumberOfConnectedRegions(const iShape* shape) const = 0;
    virtual tSigned32 getConnectedRegionFor(const iShape* shape, const cPosition& p) const = 0;
    virtual tSigned32 getConnectedRegionForAgent(iAgent* agent) const = 0;
    virtual void clearAllNamedObstacles() = 0;
    virtual bool positionIsBlockedBySurfaceTypeTraverseCosts(const iCollisionContext* context, const cPosition& position) const = 0;
    virtual tSigned32 getFederationTileIndex() const = 0;
    virtual tUnsigned32 calculatePathfindingCostForLine(const iShape* shape, const iCollisionContext* context, const cPosition& start, const cPosition& end, bool& blocked) const = 0;
    virtual void saveGroundEx(const char* format, bool includeMapping, bool includeTranslationTo2D, bool include3DPartitioning, iOutputStream& outputStream) const = 0;
    virtual iPath* findShortestPath_MultipleStartPositions(const iShape* shape, const iCollisionContext* context, cPosition const * startPositions_Buffer, tUnsigned32 startPositions_BufferEntries, const cPosition& goal, iQueryCallBack* queryCallBack) const = 0;
    virtual tSigned32 get3DFaceVertexIndexInMesh(tSigned32 faceIndex, tSigned32 vertexIndexInFace) const = 0;
    virtual void renderBaseObstacles(tSigned32 originX, tSigned32 originY, iRender3DLinesCallBack& callBack) const = 0;
    virtual void renderUnobstructedSpaceBoundaries(const iShape* shape, tSigned32 originX, tSigned32 originY, iRender3DLinesCallBack& callBack) const = 0;
    virtual void renderAgentExpansion(const iShape* expandShape, const iAgent* agent, tSigned32 originX, tSigned32 originY, iRender3DLinesCallBack& callBack) const = 0;
    virtual void renderRangeBounds(const cHorizontalRange& range, tSigned32 originX, tSigned32 originY, iRender3DLinesCallBack& callBack) const = 0;
    virtual void renderRangeBounds_Local(const cHorizontalRange& range, const cPosition& root, tSigned32 originX, tSigned32 originY, iRender3DLinesCallBack& callBack) const = 0;
    virtual cPosition positionForPointIn3DFace(tSigned32 faceIndex, tSigned32 x, tSigned32 y) const = 0;
    virtual bool pointIsIn3DFace(tSigned32 faceIndex, tSigned32 x, tSigned32 y) const = 0;
    virtual void renderUnsplitBoundaries(const iShape* shape, tSigned32 originX, tSigned32 originY, iRender3DLinesCallBack& callBack) const = 0;
    virtual void renderSplitBoundaries(const iShape* shape, tSigned32 originX, tSigned32 originY, iRender3DLinesCallBack& callBack) const = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iMesh* afterCast = static_cast<iMesh*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iPathEngine_DEFINED
#define PATHENGINE_iPathEngine_DEFINED
class iPathEngine
{
	void operator delete(void*) {}
public:
    virtual const char *const* getVersionAttributes() const = 0;
    virtual tSigned32 getInterfaceMajorVersion() const = 0;
    virtual tSigned32 getInterfaceMinorVersion() const = 0;
    virtual void getReleaseNumbers(tSigned32& majorReleaseNumber, tSigned32& minorReleaseNumber, tSigned32& internalReleaseNumber) const = 0;
    virtual void setErrorHandler(iErrorHandler* newHandler) = 0;
    virtual iErrorHandler* getErrorHandler() = 0;
    virtual void setPathPoolParameters(tSigned32 pathsPooled, tSigned32 pointsPerPath) = 0;
    virtual bool shapeIsValid(tSigned32 const * shapeCoordinates_Buffer, tUnsigned32 shapeCoordinates_BufferEntries) = 0;
    virtual iShape* newShape(tSigned32 const * shapeCoordinates_Buffer, tUnsigned32 shapeCoordinates_BufferEntries) = 0;
    virtual iMesh* loadMeshFromBuffer(const char* format, char const * data_Buffer, tUnsigned32 data_BufferEntries, const char *const* options) = 0;
    virtual iMesh* buildMeshFromContent(const iFaceVertexMesh* const * meshes_Buffer, tUnsigned32 meshes_BufferEntries, const char *const* options) = 0;
    virtual void saveContentData(const iFaceVertexMesh* const * meshes_Buffer, tUnsigned32 meshes_BufferEntries, const char* format, iOutputStream& os) = 0;
    virtual void setRandomSeed(tUnsigned32 value) = 0;
    virtual bool unobstructedSpaceVersionIsCompatible(char const * data_Buffer, tUnsigned32 data_BufferEntries) = 0;
    virtual bool connectedRegionPreprocessVersionIsCompatible(char const * data_Buffer, tUnsigned32 data_BufferEntries) = 0;
    virtual bool pathfindPreprocessVersionIsCompatible(char const * data_Buffer, tUnsigned32 data_BufferEntries) = 0;
    virtual void deleteAllObjects() = 0;
    virtual iContentChunk* newContentChunk(const iFaceVertexMesh* ground, const iAnchorsAndPinnedShapes* anchorsAndShapes, const char *const* attributes) = 0;
    virtual iContentChunk* loadContentChunk(char const * data_Buffer, tUnsigned32 data_BufferEntries) = 0;
    virtual iMeshFederation* buildMeshFederation_FromWorldMesh(const iMesh* worldMesh, tSigned32 tileSize, tSigned32 overlap, iMeshFederationTileGeneratedCallBack& tileGeneratedCallBack) = 0;
    virtual iMeshFederation* loadFederation(const char* format, char const * data_Buffer, tUnsigned32 data_BufferEntries) = 0;
    virtual iMeshFederation* buildMeshFederation_TilingOnly(const cHorizontalRange& worldRange, tSigned32 tileSize, tSigned32 overlap) = 0;
    virtual bool largeStaticObstacleShapeIsValid(tSigned32 const * shapeCoordinates_Buffer, tUnsigned32 shapeCoordinates_BufferEntries, tSigned32 x, tSigned32 y) = 0;
    virtual iSourceTiling* buildRegularSourceTiling(const cHorizontalRange& worldRange, tSigned32 tileSize) = 0;
    virtual iMesh* buildMeshFromGroundTiles(const iSourceTiling* tiling, const iMesh* const * meshes_Buffer, tUnsigned32 meshes_BufferEntries, const char *const* options) = 0;
    virtual iSourceTiling* loadSourceTiling(const char* format, char const * data_Buffer, tUnsigned32 data_BufferEntries) = 0;
    virtual void save2DContentSnapshot(const iFaceVertexMesh* const * meshes_Buffer, tUnsigned32 meshes_BufferEntries, const char *const* options, const char* format, iOutputStream& os) = 0;
    virtual iSourceTiling* buildRegularSourceTiling_RoundUpForVoxels(const cHorizontalRange& worldRange, tSigned32 tileSize, tSigned32 voxelSize) = 0;
    virtual iMesh* buildMeshByCopyingConnectedGround(const iMesh* baseMesh, cPosition const * rootPositions_Buffer, tUnsigned32 rootPositions_BufferEntries, const char *const* options) = 0;
    virtual iMesh* buildMeshAroundTiledObstructions(char const * data_Buffer, tUnsigned32 data_BufferEntries, tSigned32 tilesInX, tSigned32 tilesInY, tSigned32 originX, tSigned32 originY, tSigned32 tileSize, const char *const* options) = 0;
    virtual iMesh* buildUnobstructedSpaceGround(const iMesh* mesh, const iShape* shape, const char *const* options) = 0;
};
#endif

#ifndef PATHENGINE_iShape_DEFINED
#define PATHENGINE_iShape_DEFINED
class iShape
{
public:
    virtual void destroy() = 0;
    virtual tSigned32 size() const = 0;
    virtual void vertex(tSigned32 index, tSigned32& x, tSigned32& y) const = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iShape* afterCast = static_cast<iShape*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iCollisionContext_DEFINED
#define PATHENGINE_iCollisionContext_DEFINED
class iCollisionContext
{
public:
    virtual void destroy() = 0;
    virtual void addAgent(iAgent* agent) = 0;
    virtual void removeAgent(iAgent* agent) = 0;
    virtual tSigned32 getNumberOfAgents() const = 0;
    virtual iAgent* getAgent(tSigned32 index) const = 0;
    virtual bool includes(iAgent* agent) const = 0;
    virtual void addObstacleSet(iObstacleSet* context) = 0;
    virtual void removeObstacleSet(iObstacleSet* context) = 0;
    virtual tSigned32 getNumberOfObstacleSets() const = 0;
    virtual iObstacleSet* getObstacleSet(tSigned32 index) const = 0;
    virtual void updateCollisionPreprocessFor(const iShape* shape) const = 0;
    virtual void updatePathfindingPreprocessFor(const iShape* shape) const = 0;
    virtual void setSurfaceTypeTraverseCost(tSigned32 surfaceType, float cost) = 0;
    virtual void setSurfaceTypeTraverseCostDirection(tSigned32 surfaceType, tSigned32 directionVectorX, tSigned32 directionVectorY) = 0;
    virtual void setQueryBounds(const cHorizontalRange& bounds) = 0;
    virtual void clearQueryBounds() = 0;
    virtual void setOverlayConnectionPenalty(tSigned32 connectionIndex, tSigned32 penalty) = 0;
    virtual void clearOverlayConnectionPenalties() = 0;
    virtual bool hasOverlayConnectionPenalties() const = 0;
    virtual tSigned32 getOverlayConnectionPenalty(tSigned32 connectionIndex) const = 0;
    virtual tSigned32 addRunTimeOffMeshConnection(const cPosition& fromPosition, const cPosition& toPosition, tSigned32 forwardPenalty, tSigned32 backPenalty, tSigned32 forwardConnectionID, tSigned32 backConnectionID) = 0;
    virtual void removeRunTimeOffMeshConnection(tSigned32 connectionHandle) = 0;
    virtual void clearRunTimeOffMeshConnections() = 0;
    virtual void temporarilyIgnoreAgent(iAgent* agent) = 0;
    virtual void restoreTemporarilyIgnoredAgent(iAgent* agent) = 0;
    virtual tSigned32 getNumberOfConnectedRegions(const iShape* shape) const = 0;
    virtual tSigned32 getConnectedRegionFor(const iShape* shape, const cPosition& p) const = 0;
    virtual tSigned32 getConnectedRegionForAgent(iAgent* agent) const = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iCollisionContext* afterCast = static_cast<iCollisionContext*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iObstacleSet_DEFINED
#define PATHENGINE_iObstacleSet_DEFINED
class iObstacleSet
{
public:
    virtual void destroy() = 0;
    virtual void addAgent(iAgent* agent) = 0;
    virtual void removeAgent(iAgent* agent) = 0;
    virtual tSigned32 getNumberOfAgents() const = 0;
    virtual iAgent* getAgent(tSigned32 index) const = 0;
    virtual bool includes(iAgent* agent) const = 0;
    virtual tSigned32 size() const = 0;
    virtual bool empty() const = 0;
    virtual bool pathfindPreprocessNeedsUpdate(const iShape* shape) const = 0;
    virtual void updatePathfindPreprocessFor(const iShape* shape) const = 0;
    virtual void savePathfindPreprocessFor(const iShape* shape, iOutputStream& os) const = 0;
    virtual void loadPathfindPreprocessFor(const iShape* shape, char const * data_Buffer, tUnsigned32 data_BufferEntries) const = 0;
    virtual bool connectedRegionPreprocessNeedsUpdate(const iShape* shape) const = 0;
    virtual void updateConnectedRegionPreprocessFor(const iShape* shape) const = 0;
    virtual tSigned32 getNumberOfConnectedRegions(const iShape* shape) const = 0;
    virtual tSigned32 getConnectedRegionFor(const iShape* shape, const cPosition& p) const = 0;
    virtual tSigned32 getConnectedRegionForAgent(iAgent* agent) const = 0;
    virtual void clear() = 0;
    virtual void temporarilyIgnoreAgent(iAgent* agent) = 0;
    virtual void restoreTemporarilyIgnoredAgent(iAgent* agent) = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iObstacleSet* afterCast = static_cast<iObstacleSet*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iPath_DEFINED
#define PATHENGINE_iPath_DEFINED
class iPath
{
public:
    virtual void destroy() = 0;
    virtual tSigned32 size() const = 0;
    virtual cPosition position(tSigned32 pointIndex) const = 0;
    virtual tSigned32 connectionIndex(tSigned32 segmentIndex) const = 0;
    virtual const cPosition* getPositionArray() const = 0;
    virtual const tSigned32* getConnectionIndexArray() const = 0;
    virtual tUnsigned32 getLength() const = 0;
    virtual iCollisionInfo* advanceAlong(const iShape* shape, float distance, const iCollisionContext* context, float& precisionX, float& precisionY) = 0;
    virtual void renderOnGround(tSigned32 originX, tSigned32 originY, iRender3DLinesCallBack& callBack) const = 0;
    virtual iMesh* getMesh() const = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iPath* afterCast = static_cast<iPath*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iCollisionInfo_DEFINED
#define PATHENGINE_iCollisionInfo_DEFINED
class iCollisionInfo
{
public:
    virtual void destroy() = 0;
    virtual void getCollidingLine(tSigned32& startX, tSigned32& startY, tSigned32& endX, tSigned32& endY) const = 0;
    virtual iAgent* getCollidingAgent() const = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iCollisionInfo* afterCast = static_cast<iCollisionInfo*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iTerrainCallBack_DEFINED
#define PATHENGINE_iTerrainCallBack_DEFINED
class iTerrainCallBack
{
public:
    virtual ~iTerrainCallBack() {}
    virtual float getHeightAt(tSigned32 x, tSigned32 y) = 0;
    virtual float getHeightAtWithPrecision(tSigned32 x, tSigned32 y, float precisionX, float precisionY) = 0;
};
#endif

#ifndef PATHENGINE_iRender3DLinesCallBack_DEFINED
#define PATHENGINE_iRender3DLinesCallBack_DEFINED
class iRender3DLinesCallBack
{
public:
    virtual ~iRender3DLinesCallBack() {}
    virtual void startVertex(tSigned32 terrainLayer, float x, float y, float z) = 0;
    virtual void vertex(float x, float y, float z) = 0;
};
#endif

#ifndef PATHENGINE_iQueryCallBack_DEFINED
#define PATHENGINE_iQueryCallBack_DEFINED
class iQueryCallBack
{
public:
    virtual ~iQueryCallBack() {}
    virtual tSigned32 desiredCallBackFrequency() = 0;
    virtual bool abort() = 0;
};
#endif

#ifndef PATHENGINE_iContentChunk_DEFINED
#define PATHENGINE_iContentChunk_DEFINED
class iContentChunk
{
public:
    virtual void destroy() = 0;
    virtual bool preValidate(tSigned32 maximumScale, tSigned32 translationRange) = 0;
    virtual void save(iOutputStream& outputStream) const = 0;
    virtual iContentChunkInstance* instantiate(tSigned32 rotation, tSigned32 scale, tSigned32 translationX, tSigned32 translationY, float translationZ, tSigned32 sectionID) = 0;
    virtual iContentChunkInstance* instantiate_FloatRotateAndScale(float rotation, float scale, tSigned32 translationX, tSigned32 translationY, float translationZ, tSigned32 sectionID) = 0;
    virtual void deleteAllInstances() = 0;
    virtual bool hasGroundComponent() const = 0;
    virtual bool hasAnchorsAndShapesComponent() const = 0;
    virtual tSigned32 numberOfConnectingEdges() const = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iContentChunk* afterCast = static_cast<iContentChunk*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iContentChunkInstance_DEFINED
#define PATHENGINE_iContentChunkInstance_DEFINED
class iContentChunkInstance
{
public:
    virtual void destroy() = 0;
    virtual iContentChunk* getContentChunk() const = 0;
    virtual const iFaceVertexMesh* getGroundPointer() const = 0;
    virtual const iAnchorsAndPinnedShapes* getAnchorsAndShapesPointer() const = 0;
    virtual void addAnchorsAndShapes(iMesh* mesh, const char* idPrefix, tSigned32 numberOfTerrainLayers) const = 0;
    virtual void getHorizontalRange(cHorizontalRange& result) const = 0;
    virtual tSigned32 numberOfConnectingEdges() const = 0;
    virtual void getConnectingEdgeGeometry(tSigned32 connectingEdgeIndex, tSigned32& startX, tSigned32& startY, float& startZ, tSigned32& endX, tSigned32& endY, float& endZ) const = 0;
    virtual bool edgesCanConnect(tSigned32 connectingEdgeOnThis, iContentChunkInstance* targetInstance, tSigned32 connectingEdgeOnTarget) const = 0;
    virtual void makeEdgeConnection(tSigned32 connectingEdgeOnThis, iContentChunkInstance* targetInstance, tSigned32 connectingEdgeOnTarget) const = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iContentChunkInstance* afterCast = static_cast<iContentChunkInstance*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iMeshFederation_DEFINED
#define PATHENGINE_iMeshFederation_DEFINED
class iMeshFederation
{
public:
    virtual void destroy() = 0;
    virtual void save(const char* format, iOutputStream& outputStream) const = 0;
    virtual tSigned32 size() const = 0;
    virtual bool coordinatesAreInsideRange(tSigned32 x, tSigned32 y) const = 0;
    virtual tSigned32 tileForQuery(tSigned32 queryStartX, tSigned32 queryStartY) const = 0;
    virtual void getTileCentre(tSigned32 tileIndex, tSigned32& centreX, tSigned32& centreY) const = 0;
    virtual void getHandledRegion_World(tSigned32 tileIndex, cHorizontalRange& result) const = 0;
    virtual void getRepresentedRegion_World(tSigned32 tileIndex, cHorizontalRange& result) const = 0;
    virtual void getHandledRegion_Local(tSigned32 tileIndex, cHorizontalRange& result) const = 0;
    virtual void getRepresentedRegion_Local(tSigned32 tileIndex, cHorizontalRange& result) const = 0;
    virtual cPosition translatePosition(tSigned32 fromTileIndex, const iMesh* fromMesh, tSigned32 toTileIndex, const iMesh* toMesh, const cPosition& fromPosition) const = 0;
    virtual tSigned32 getNumberOfRepresentedRegionsOverlapped(const cHorizontalRange& queryRegion) const = 0;
    virtual void getRepresentedRegionsOverlapped(const cHorizontalRange& queryRegion, iArrayReceiver<tSigned32>& results) const = 0;
    virtual iMesh* buildTileMeshFromContent(tSigned32 tileIndex, const iContentChunkInstance* const * chunks_Buffer, tUnsigned32 chunks_BufferEntries, const char *const* options, iOutputStream& os) const = 0;
    virtual tSigned32 getNumberOfGroundTilesOverlapped(tSigned32 tileIndex, const iSourceTiling* tiling) const = 0;
    virtual void getGroundTilesOverlapped(tSigned32 tileIndex, const iSourceTiling* tiling, iArrayReceiver<tSigned32>& results) const = 0;
    virtual iMesh* buildTileMeshFromLocalGroundTiles(tSigned32 tileIndex, const iSourceTiling* tiling, const iMesh* const * meshes_Buffer, tUnsigned32 meshes_BufferEntries, const char *const* options) const = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iMeshFederation* afterCast = static_cast<iMeshFederation*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iSourceTiling_DEFINED
#define PATHENGINE_iSourceTiling_DEFINED
class iSourceTiling
{
public:
    virtual void destroy() = 0;
    virtual tSigned32 size() const = 0;
    virtual void getTileRange(tSigned32 tileIndex, cHorizontalRange& result) const = 0;
    virtual const char* getTileRangeAsString(tSigned32 tileIndex) const = 0;
    virtual void getTileFilterRange(tSigned32 tileIndex, cHorizontalRange& result) const = 0;
    virtual void save(const char* format, iOutputStream& outputStream) const = 0;
    virtual void getWorldRange(cHorizontalRange& result) const = 0;
    virtual void getTileFilterRange_Voxels(tSigned32 tileIndex, tSigned32 voxelSize, cHorizontalRange& result) const = 0;
	void operator delete(void* voidPointer)
	{
		if(voidPointer)
		{
			iSourceTiling* afterCast = static_cast<iSourceTiling*>(voidPointer);
            afterCast->destroy();
        }
    }
};
#endif

#ifndef PATHENGINE_iAnchorsAndPinnedShapes_DEFINED
#define PATHENGINE_iAnchorsAndPinnedShapes_DEFINED
class iAnchorsAndPinnedShapes
{
public:
    virtual ~iAnchorsAndPinnedShapes() {}
    virtual tSigned32 numberOfAnchors() const = 0;
    virtual const char* anchor_Name(tSigned32 anchorIndex) const = 0;
    virtual void anchor_HorizontalPosition(tSigned32 anchorIndex, tSigned32& x, tSigned32& y) const = 0;
    virtual void anchor_VerticalRange(tSigned32 anchorIndex, float& z_Start, float& z_End) const = 0;
    virtual tSigned32 anchor_Orientation(tSigned32 anchorIndex) const = 0;
    virtual tSigned32 anchor_Attribute(tSigned32 anchorIndex, tSigned32 attributeIndex) const = 0;
    virtual tSigned32 numberOfPinnedShapes() const = 0;
    virtual const char* pinnedShape_Name(tSigned32 shapeIndex) const = 0;
    virtual tSigned32 pinnedShape_AnchorIndex(tSigned32 shapeIndex) const = 0;
    virtual tSigned32 pinnedShape_NumberOfVertices(tSigned32 shapeIndex) const = 0;
    virtual void pinnedShape_Vertex(tSigned32 shapeIndex, tSigned32 vertexIndex, tSigned32& x, tSigned32& y) const = 0;
    virtual tSigned32 pinnedShape_Attribute(tSigned32 shapeIndex, tSigned32 attributeIndex) const = 0;
};
#endif

#ifndef PATHENGINE_iSolidObjects_DEFINED
#define PATHENGINE_iSolidObjects_DEFINED
class iSolidObjects
{
public:
    virtual ~iSolidObjects() {}
    virtual tSigned32 numberOfConvexSolids() const = 0;
    virtual tSigned32 convexSolid_Points(tSigned32 convexSolidIndex) const = 0;
    virtual void convexSolid_Point(tSigned32 convexSolidIndex, tSigned32 pointIndex, tSigned32& x, tSigned32& y, tSigned32& z) const = 0;
    virtual tSigned32 convexSolid_Attribute(tSigned32 convexSolidIndex, tSigned32 attributeIndex) const = 0;
};
#endif

#ifndef PATHENGINE_iMeshFederationTileGeneratedCallBack_DEFINED
#define PATHENGINE_iMeshFederationTileGeneratedCallBack_DEFINED
class iMeshFederationTileGeneratedCallBack
{
public:
    virtual ~iMeshFederationTileGeneratedCallBack() {}
    virtual void tileGenerated(tSigned32 tileIndex, iMesh* tileMesh) = 0;
};
#endif

#ifndef PATHENGINE_iConnectionTestCallBack_DEFINED
#define PATHENGINE_iConnectionTestCallBack_DEFINED
class iConnectionTestCallBack
{
public:
    virtual ~iConnectionTestCallBack() {}
    virtual void testJumpCandidatePair(tSigned32 fromX, tSigned32 fromY, tSigned32 fromZ, tSigned32 toX, tSigned32 toY, tSigned32 toZ, tSigned32& penaltyOut, tSigned32& penaltyBack) = 0;
    virtual void testDropCandidatePair(tSigned32 fromX, tSigned32 fromY, tSigned32 fromZ, tSigned32 toX, tSigned32 toY, tSigned32 toZ, tSigned32& penaltyOut, tSigned32& penaltyBack) = 0;
};
#endif

#ifndef PATHENGINE_iProgressCallBack_DEFINED
#define PATHENGINE_iProgressCallBack_DEFINED
class iProgressCallBack
{
public:
    virtual ~iProgressCallBack() {}
    virtual void updateProgress(const char* operationDescription, float percent) = 0;
};
#endif

#ifndef PATHENGINE_iErrorHandler_DEFINED
#define PATHENGINE_iErrorHandler_DEFINED
class iErrorHandler
{
public:
    virtual ~iErrorHandler() {}
    virtual tSigned32 handle(const char* type, const char* description, const char *const* attributes) = 0;
};
#endif

static const tSigned32 PE_FaceAttribute_SurfaceType = 0;
static const tSigned32 PE_FaceAttribute_SectionID = 1;
static const tSigned32 PE_FaceAttribute_MarksPortal = 2;
static const tSigned32 PE_FaceAttribute_CP3D_ExcludeFromGroundResult = 2;
static const tSigned32 PE_FaceAttribute_MarksExternalShape = 3;
static const tSigned32 PE_FaceAttribute_CP3D_ExcludeFromSteepSlopeFilter = 3;
static const tSigned32 PE_FaceAttribute_MarksConnectingEdge = 4;
static const tSigned32 PE_FaceAttribute_UserData = 5;
static const tSigned32 PE_FaceAttribute_MarksTerrainPortal = 6;
static const tSigned32 PE_FaceAttribute_Top = 6;

static const tSigned32 PE_AnchorAttribute_ResolvesToTerrain = 0;
static const tSigned32 PE_AnchorAttribute_Top = 0;
static const tSigned32 PE_PinnedShapeAttribute_Type = 0;
static const tSigned32 PE_PinnedShapeAttribute_Top = 0;
static const tSigned32 PE_PinnedShapeType_BurntIn = 0;
static const tSigned32 PE_PinnedShapeType_NamedObstacle = 1;
static const tSigned32 PE_PinnedShapeType_Top = 1;

static const tSigned32 PE_ErrorHandler_Continue = 0;
static const tSigned32 PE_ErrorHandler_Abort = 1;
static const tSigned32 PE_ErrorHandler_Break = 2;
static const tSigned32 PE_ErrorHandler_ContinueAndIgnoreInFuture = 3;

#ifndef PATHENGINE_I_ALLOCATOR_DEFINED
#define PATHENGINE_I_ALLOCATOR_DEFINED
class iAllocator
{
public:
    virtual ~iAllocator() {}
    virtual void* allocate(tUnsigned32 size) = 0;
    virtual void deallocate(void* ptr) = 0;
    virtual void* expand(void* oldPtr, tUnsigned32 oldSize, tUnsigned32 oldSize_Used, tUnsigned32 newSize) = 0;
// helper
    template <class T> void
    allocate_Array(tUnsigned32 arraySize, T*& result)
    {
        result = static_cast<T*>(allocate(sizeof(T) * arraySize));
    }
    template <class T> void
    expand_Array(T*& ptr, tUnsigned32 oldArraySize, tUnsigned32 oldArraySize_Used, tUnsigned32 newArraySize)
    {
        ptr = static_cast<T*>(expand(ptr, sizeof(T) * oldArraySize, sizeof(T) * oldArraySize_Used, sizeof(T) * newArraySize));
    }
};
#endif //ndef PATHENGINE_I_ALLOCATOR_DEFINED

#endif // ends file include guard
