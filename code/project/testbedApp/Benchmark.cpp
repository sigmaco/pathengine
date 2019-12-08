
#include "base/Header.h"
#include "project/testbedApp/Benchmark/Benchmark_LongPaths.h"
#include "project/testbedApp/Benchmark/Benchmark_DynamicObstaclesPathfinding.h"
#include "sampleShared/Error.h"
#include "platform_common/TestbedApplicationEntryPoint.h"
#include "externalAPI/i_testbed.h"
#include <time.h>
#include <fstream>
#include <string>
#include <vector>

static iMesh*
LoadMeshAndGeneratePreprocess(
        iPathEngine* pathEngine, iTestBed* testbed,
        iShape* shape,
        const std::string& meshName,
        const char** loadOptions,
        const char** unobstructedSpaceAttributes,
        std::ofstream& os, 
        const char* meshDescription = 0
        )
{
    if(!meshDescription)
    {
        meshDescription = meshName.c_str();
    }
    os << "** benchmarks with mesh '" << meshDescription << "'\n";
    iMesh* mesh;
    clock_t start, finish;
    double  duration;
    {
        char* buffer;
        tUnsigned32 size;
        std::string meshPath = "../resource/meshes/" + meshName + ".tok";
        buffer = testbed->loadBinary(meshPath.c_str(), size);
        start = clock();
        mesh = pathEngine->loadMeshFromBuffer("tok", buffer, size, loadOptions);
        finish = clock();
        testbed->freeBuffer(buffer);
        duration = static_cast<double>(finish - start) / CLOCKS_PER_SEC;
        os << "loading mesh from buffer: " << duration << " seconds\n";
    }
    {
        std::string message = "generating preprocess for: '" + meshName + "'";
        testbed->setColour("orange");
        testbed->printTextLine(10, message.c_str());
        testbed->update_ExitIfRequested();
    }
    {
        start = clock();
        mesh->generateUnobstructedSpaceFor(shape, true, unobstructedSpaceAttributes);
        finish = clock();
        duration = static_cast<double>(finish - start) / CLOCKS_PER_SEC;
        os << "generating unobstructed space: " << duration << " seconds\n";
    }
    {
        start = clock();
        mesh->generatePathfindPreprocessFor(shape, 0);
        finish = clock();
        duration = static_cast<double>(finish - start) / CLOCKS_PER_SEC;
        os << "generating pathfind preprocess: " << duration << " seconds\n";
    }
    return mesh;
}

void
TestbedApplicationMain(iPathEngine* pathEngine, iTestBed* testBed)
{
// check if interfaces are compatible with the headers used for compilation
    if(testBed->getInterfaceMajorVersion()!=TESTBED_INTERFACE_MAJOR_VERSION
        ||
        testBed->getInterfaceMinorVersion()<TESTBED_INTERFACE_MINOR_VERSION)
    {
        testBed->reportError("Fatal","Testbed version is incompatible with headers used for compilation.");
        return;
    }
    if(pathEngine->getInterfaceMajorVersion()!=PATHENGINE_INTERFACE_MAJOR_VERSION
        ||
        pathEngine->getInterfaceMinorVersion()<PATHENGINE_INTERFACE_MINOR_VERSION)
    {
        testBed->reportError("Fatal","Pathengine version is incompatible with headers used for compilation.");
        return;
    }
    
    std::ofstream os("benchmark_results.txt");
    iShape* shape;
    {
        tSigned32 array[]=
        {
            -20, 20,
            20, 20,
            20, -20,
            -20, -20,
        };
        shape = pathEngine->newShape(array, sizeof(array) / sizeof(*array));
    }
    iShape* bigShape;
    {
        tSigned32 array[]=
        {
            -100, 100,
            100, 100,
            100, -100,
            -100, -100,
        };
        bigShape = pathEngine->newShape(array, sizeof(array) / sizeof(*array));
    }
    iMesh* mesh;
    mesh = LoadMeshAndGeneratePreprocess(pathEngine, testBed, shape, "mesh1", 0, 0, os);
    pathEngine->setRandomSeed(0); // random seed value required to generate same sequence as historic benchmark results
    Benchmark_LongPaths(testBed, mesh, shape, os);
    delete mesh;
    mesh = LoadMeshAndGeneratePreprocess(pathEngine, testBed, shape, "flatshead_dungeon", 0, 0, os);
    pathEngine->setRandomSeed(1391971626UL); // random seed value required to generate same sequence as historic benchmark results
    Benchmark_LongPaths(testBed, mesh, shape, os);
    Benchmark_DynamicObstaclesPathfinding(
            pathEngine, testBed, mesh,
            shape, // agent shape
            shape, // obstacle shape
            5,  // number of dynamic obstacles
            1000,  // number of collision context iterations
            10,  // number of pathfinding queries per collision context iteration
            333, // random seed
            os
            );
    Benchmark_DynamicObstaclesPathfinding(
            pathEngine, testBed, mesh,
            shape, // agent shape
            shape, // obstacle shape
            10,  // number of dynamic obstacles
            1000,  // number of collision context iterations
            10,  // number of pathfinding queries per collision context iteration
            333, // random seed
            os
            );
    Benchmark_DynamicObstaclesPathfinding(
            pathEngine, testBed, mesh,
            shape, // agent shape
            shape, // obstacle shape
            20,  // number of dynamic obstacles
            1000,  // number of collision context iterations
            10,  // number of pathfinding queries per collision context iteration
            333, // random seed
            os
            );
    delete mesh;
    {
        const char* attributes[] = {
            "splitWithCircumferenceBelow", "2000",
            0
        };
        mesh = LoadMeshAndGeneratePreprocess(pathEngine, testBed, shape, "thainesford", 0, attributes, os);
    }
    pathEngine->setRandomSeed(523518096UL); // random seed value required to generate same sequence as historic benchmark results
    Benchmark_LongPaths(testBed, mesh, shape, os);
    Benchmark_DynamicObstaclesPathfinding(
            pathEngine, testBed, mesh,
            shape, // agent shape
            bigShape, // obstacle shape
            5,  // number of dynamic obstacles
            1000,  // number of collision context iterations
            10,  // number of pathfinding queries per collision context iteration
            333, // random seed
            os
            );
    Benchmark_DynamicObstaclesPathfinding(
            pathEngine, testBed, mesh,
            shape, // agent shape
            bigShape, // obstacle shape
            10,  // number of dynamic obstacles
            1000,  // number of collision context iterations
            10,  // number of pathfinding queries per collision context iteration
            333, // random seed
            os
            );
    Benchmark_DynamicObstaclesPathfinding(
            pathEngine, testBed, mesh,
            shape, // agent shape
            bigShape, // obstacle shape
            20,  // number of dynamic obstacles
            1000,  // number of collision context iterations
            10,  // number of pathfinding queries per collision context iteration
            333, // random seed
            os
            );
    delete mesh;
    {
        const char* attributes[] = {
            "splitWithCircumferenceBelow", "2000",
            "optimiseWithRange", "18",
            0
        };
        mesh = LoadMeshAndGeneratePreprocess(pathEngine, testBed, shape, "thainesford", 0, attributes, os, "thainesford (optimised)");
    }
    pathEngine->setRandomSeed(1);
    Benchmark_LongPaths(testBed, mesh, shape, os);
    Benchmark_DynamicObstaclesPathfinding(
            pathEngine, testBed, mesh,
            shape, // agent shape
            bigShape, // obstacle shape
            5,  // number of dynamic obstacles
            1000,  // number of collision context iterations
            10,  // number of pathfinding queries per collision context iteration
            333, // random seed
            os
            );
    Benchmark_DynamicObstaclesPathfinding(
            pathEngine, testBed, mesh,
            shape, // agent shape
            bigShape, // obstacle shape
            10,  // number of dynamic obstacles
            1000,  // number of collision context iterations
            10,  // number of pathfinding queries per collision context iteration
            333, // random seed
            os
            );
    Benchmark_DynamicObstaclesPathfinding(
            pathEngine, testBed, mesh,
            shape, // agent shape
            bigShape, // obstacle shape
            20,  // number of dynamic obstacles
            1000,  // number of collision context iterations
            10,  // number of pathfinding queries per collision context iteration
            333, // random seed
            os
            );
    delete mesh;
}
