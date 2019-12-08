using System;
using PathEngine;

class ConsoleErrorHandler : ErrorHandler
{
    int ErrorHandler.handle(string type, string description, string[] attributes)
    {
        Console.WriteLine("Got an error from PathEngine!");
        Console.WriteLine("type = {0}", type);
        Console.WriteLine("description = {0}", description);
        for(int i = 0; i != attributes.Length; i += 2)
        {
            Console.WriteLine("error attribute '{0}' = '{1}'", attributes[i], attributes[i + 1]);
        }
        Console.WriteLine("Press enter to continue.");
        Console.ReadLine();
        return Constants.ErrorHandler_Continue;
    }
};

class SquareMesh : FaceVertexMesh
{
    int[] vertIndices = { 0, 1, 2, 0, 2, 3 };
    int FaceVertexMesh.vertices()
    {
        return 4;
    }
    int FaceVertexMesh.faces()
    {
        return vertIndices.Length / 3;
    }
    int FaceVertexMesh.vertexIndex(int face, int vertInFace)
    {
        return vertIndices[face * 3 + vertInFace];
    }
    int FaceVertexMesh.vertexX(int v)
    {
        if(v < 2)
        {
            return 0;
        }
        return 100;
    }
    int FaceVertexMesh.vertexY(int v)
    {
        if (v == 0 || v == 3)
        {
            return 0;
        }
        return 100;
    }
    float FaceVertexMesh.vertexZ(int v)
    {
        return 0;
    }
    int FaceVertexMesh.faceAttribute(int face, int attributeIndex)
    {
        return -1;
    }
};

namespace ConsoleApplication1
{
    class Program
    {
        static void Main(string[] args)
        {
        // supplying a custom error handler to SDK initialisation

            // note that a simple messagebox error handler will be supplied, by default
            // but you should probably provide a custom error callback that interfaces directly with application side error reporting and logging functionality
            // this example code shows how to implement a custom error callback that directs error information to console output

            PathEngine.PathEngine pathEngine = DLLManagement.loadDLL("PathEngine", new ConsoleErrorHandler());

       // querying SDK version attributes

            {
                int major, minor, internalRelease;
                pathEngine.getReleaseNumbers(out major, out minor, out internalRelease);
                Console.WriteLine("getReleaseNumbers returns {0}.{1}.{2}", major, minor, internalRelease);
            }

            String[] versionAttributes = pathEngine.getVersionAttributes();
            Console.WriteLine("version attributes:");
            for(int i = 0; i < versionAttributes.Length; i += 2)
            {
                Console.WriteLine("{0} = {1}", versionAttributes[i], versionAttributes[i + 1]);
            }
            Console.WriteLine("(end of version attributes)");

        // provoke a non fatal error, and null object return

            // The error handling philosophy in PathEngine is that code should be set up so that errors should not occur,
            // so the API provides methods to validate data where relevant, and where data is validated correctly you shouldn't see any errors.

            int[] shapeCoords = {-10,-10, -10,10, 10,10, 10,-10};
            shapeCoords[0] = 100; // bad coordinate

            bool valid = pathEngine.shapeIsValid(shapeCoords);
            Console.WriteLine("shapeIsValid returns {0}", valid);

            // The following call to newShape() is not considered good coding practice, then.
            // Instead application code should use shapeIsValid() (as above) to ensure that newShape() isn't called with bad coordinates.
            // But we use this here to provoke a 'NonFatal' error, so that we can see this being passed to the application side error handler.
            // In this case PathEngine will handle the error gracefully, and you should get an error posted to the supplied error handler callback, and a null object returned,
            // but in other cases, PathEngine may emit 'Fatal' errors on invalid data, and then crash after the error callback returns!

            Shape shape = pathEngine.newShape(shapeCoords); // will generate a NonFatal Error
            Console.WriteLine("shape.isNull() returns {0}", shape.isNull());

            shapeCoords[0] = -10; // fix the bad coordinate
            Console.WriteLine("(coordinates fixed)");

            valid = pathEngine.shapeIsValid(shapeCoords);
            Console.WriteLine("shapeIsValid returns {0}", valid);

            shape = pathEngine.newShape(shapeCoords);
            Console.WriteLine("shape.isNull() returns {0}", shape.isNull());

        // using the FaceVertexMesh callback to generate a simple square ground mesh

            FaceVertexMesh[] contentMeshes = new FaceVertexMesh[1];
            contentMeshes[0] = new SquareMesh();
            Mesh mesh = pathEngine.buildMeshFromContent(contentMeshes, new String[0]);
            Console.WriteLine("mesh built from content, getNumberOf3DFaces returns {0}", mesh.getNumberOf3DFaces());

        // this time we generate unobstructed space preprocess only
        // (this enables collision queries for the agent shape, pathfind preprocess is not required for the following)

            mesh.generateUnobstructedSpaceFor(shape, false, new String[0]);

        // place an agent and add to a collision context

            Position pos = mesh.generateRandomPosition();
            Agent placedAgent = mesh.placeAgent(shape, pos);
            CollisionContext context = mesh.newContext();
            context.addAgent(placedAgent);
            Console.WriteLine("context getNumberOfAgents() returns {0}", context.getNumberOfAgents());

        // call getAllAgentsOverlapped
        // (this shows the linkage for an array out argument, and also gives as another handle to compare to placedAgent)

            Agent[] overlapped;
            mesh.getAllAgentsOverlapped(shape, context, pos, out overlapped);
            Console.WriteLine("number of agents overlapped = {0}", overlapped.Length);

        // we can't compare interface object handles directly, but a getComparisonToken() method is supplied for this purpose
        // (and can also be used for sorting, if this is required)

            Console.WriteLine("placedAgent.getComparisonToken() returns {0}", placedAgent.getComparisonToken());
            foreach(Agent overlappedAgent in overlapped)
            {
                Console.WriteLine("overlappedAgent.getComparisonToken() returns {0}", overlappedAgent.getComparisonToken());
            }

        // a simple file output stream callback is supplied

            OutputStream os = new FileOutputStream("savedMesh.xml");
            mesh.saveGround("xml", false, os);

        // object lifetime notes

            {
                Mesh mesh2 = pathEngine.buildMeshFromContent(contentMeshes, new String[0]);
            // ** PathEngine interface objects are not garbage collected, so mesh2 will not be cleaned up automatically
            // ** and will therefore now be leaked, unless pathEngine.deleteAllObjects() is called
            }
            mesh.destroy(); // clean up this interface object explicitly
            // ** but don't forget that the interface object is now no longer valid, and methods should not be called on this object
            // ** and note that dependant objects such as placed agents and collision contexts are also destroyed with the mesh
            pathEngine.deleteAllObjects(); // cleans up all objects created by PathEngine

            Console.WriteLine("(completed, press enter)");
            Console.ReadLine();
        }
    }
}