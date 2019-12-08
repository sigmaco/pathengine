using System;
using PathEngine;

namespace ConsoleApplication1
{
    class Program
    {
        static void Main(string[] args)
        {
            PathEngine.PathEngine pathEngine = DLLManagement.loadDLL("PathEngine");

        // load a ground mesh

            byte[] meshBuffer = System.IO.File.ReadAllBytes("../resource/meshes/mesh1.xml");
            string[] loadingOptions = {};
            Mesh mesh = pathEngine.loadMeshFromBuffer("xml", meshBuffer, loadingOptions);

        // create a pathfinding agent shape

            Shape shape;
            {
                int[] coordinateData = { -10, -10, -10, 10, 10, 10, 10, -10 };
                shape = pathEngine.newShape(coordinateData);
            }

        // generate preprocess for this agent shape

            string[] noOptions = { };
            mesh.generateUnobstructedSpaceFor(shape, true, noOptions);
            mesh.generatePathfindPreprocessFor(shape, noOptions);

        // generate random unobstructed positions for query start and end

            // collision contexts provide dynamic state
            // default constructed API object references act like null pointers to API objects in the native mapping
            // and null collision context arguments are used in the API to indicate no dynamic state
            CollisionContext nullContext;
            Position start;
            do
            {
                 start = mesh.generateRandomPosition();
            }
            while(mesh.testPointCollision(shape, nullContext, start));
            Console.WriteLine("start: {0}:{1},{2})", start.cell, start.x, start.y);
            Position goal;
            do
            {
                 goal = mesh.generateRandomPosition();
            }
            while(mesh.testPointCollision(shape, nullContext, goal));
            Console.WriteLine("goal: {0}:{1},{2})", goal.cell, goal.x, goal.y);

        // pathfind!

            Path path = mesh.findShortestPath(shape, nullContext, start, goal);
            if(path.isNull())
            {
                Console.WriteLine("No unobstructed path exists between start and goal.");
            }
            else
            {
                Console.WriteLine("Path found with {0} points:", path.size());
                for(int i = 0; i < path.size(); ++i)
                {
                    Position p = path.position(i);
                    Console.WriteLine("{0}:{1},{2}", p.cell, p.x, p.y);
                }
            }

            Console.WriteLine("(completed, press enter)");
            Console.ReadLine();
        }
    }
}