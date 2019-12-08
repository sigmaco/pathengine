Imports PathEngine

Module Module1

    ' call back implementations

    Public Class ConsoleErrorHandler
        Implements ErrorHandler
        Function handle(errorType As String, errorDescription As String, attributes() As String) As Integer Implements ErrorHandler.handle
            Console.WriteLine("Got an error from PathEngine!")
            Console.WriteLine("type = {0}", errorType)
            Console.WriteLine("description = {0}", errorDescription)
            For i As Integer = LBound(attributes) To UBound(attributes) Step 2
                Console.WriteLine("error attribute '{0}' = '{1}'", attributes(i), attributes(i + 1))
            Next
            Console.WriteLine("Press enter to continue.")
            Console.ReadLine()
            Return Constants.ErrorHandler_Continue
        End Function
    End Class

    Public Class SquareMesh
        Implements FaceVertexMesh
        Dim vertIndices As Integer() = {0, 1, 2, 0, 2, 3}
        Function vertices() As Integer Implements FaceVertexMesh.vertices
            Return 4
        End Function
        Function faces() As Integer Implements FaceVertexMesh.faces
            Return vertIndices.Length / 3
        End Function
        Function vertexIndex(ByVal face As Integer, ByVal vertInFace As Integer) As Integer Implements FaceVertexMesh.vertexIndex
            Return vertIndices(face * 3 + vertInFace)
        End Function
        Function vertexX(ByVal v As Integer) As Integer Implements FaceVertexMesh.vertexX
            If v < 2 Then
                Return 0
            End If
            Return 100
        End Function
        Function vertexY(ByVal v As Integer) As Integer Implements FaceVertexMesh.vertexY
            If v = 0 Or v = 3 Then
                Return 0
            End If
            Return 100
        End Function
        Function vertexZ(ByVal v As Integer) As Single Implements FaceVertexMesh.vertexZ
            Return 0
        End Function
        Function faceAttribute(ByVal face As Integer, ByVal attributeIndex As Integer) As Integer Implements FaceVertexMesh.faceAttribute
            Return -1
        End Function
    End Class

    Sub Main()
        'Supplying a custom error handler to SDK initialisation

        'note that a simple messagebox error handler will be supplied, by default
        'but you should probably provide a custom error callback that interfaces directly with application side error reporting and logging functionality
        'this example code shows how to implement a custom error callback that directs error information to console output

        Dim pathEngine As PathEngine.PathEngine
        pathEngine = DLLManagement.loadDLL("PathEngine", New ConsoleErrorHandler())

        'querying SDK version attributes

        Dim major, minor, internalRelease As Integer
        pathEngine.getReleaseNumbers(major, minor, internalRelease)
        Console.WriteLine("release version = {0}.{1:D2}.{2:D2}", major, minor, internalRelease)
        Console.WriteLine("version attributes:")
        Dim versionAttributes As String() = pathEngine.getVersionAttributes()
        For i As Integer = LBound(versionAttributes) To UBound(versionAttributes) Step 2
            Console.WriteLine("'{0}' = '{1}'", versionAttributes(i), versionAttributes(i + 1))
        Next
        Console.WriteLine("(end of version attributes)")

        'Provoke a non fatal error, and null object return

        'note that the error handling philosophy in PathEngine is that code should be set up so that errors should not occur,
        'so the API provides methods to validate data where relevant, and where data is validated correctly you shouldn't see any errors.

        Dim shapeCoords As Integer() = {-10, -10, -10, 10, 10, 10, 10, -10}
        shapeCoords(0) = 100 'bad coordinate

        Dim valid As Boolean
        valid = pathEngine.shapeIsValid(shapeCoords)
        Console.WriteLine("shapeIsValid returns {0}", valid)

        'the following call to newShape() is not considered good coding practice, then.
        'instead application code should use shapeIsValid() (as above) to ensure that newShape() isn't called with bad coordinates.
        'but we use this here to provoke a 'NonFatal' error, so that we can see this being passed to the application side error handler.
        'in this case PathEngine will handle the error gracefully, and you should get an error posted to the supplied error handler callback, and a null object returned,
        'but in other cases, PathEngine may emit 'Fatal' errors on invalid data, and then crash after the error callback returns!

        Dim shape As Shape = pathEngine.newShape(shapeCoords) 'will generate a NonFatal Error
        Console.WriteLine("shape.isNull() returns {0}", shape.isNull())

        shapeCoords(0) = -10 'fix the bad coordinate
        Console.WriteLine("(coordinates fixed)")

        valid = pathEngine.shapeIsValid(shapeCoords)
        Console.WriteLine("shapeIsValid returns {0}", valid)

        shape = pathEngine.newShape(shapeCoords)
        Console.WriteLine("shape.isNull() returns {0}", shape.isNull())


        'Using the FaceVertexMesh callback to generate a simple square ground mesh

        Dim contentMeshes As FaceVertexMesh() = {New SquareMesh()}
        Dim noOptions As String() = {}
        Dim mesh As Mesh = pathEngine.buildMeshFromContent(contentMeshes, noOptions)
        Console.WriteLine("mesh built from content, getNumberOf3DFaces returns {0}", mesh.getNumberOf3DFaces())

        'preprocess generation
        'this time we generate unobstructed space preprocess only
        '(this enables collision queries for the agent shape, pathfind preprocess is not required for the following)

        mesh.generateUnobstructedSpaceFor(shape, False, noOptions)

        'place an agent and add to a collision context

        Dim pos As Position = mesh.generateRandomPosition()
        Dim placedAgent As Agent = mesh.placeAgent(shape, pos)
        Dim context As CollisionContext = mesh.newContext()
        context.addAgent(placedAgent)
        Console.WriteLine("context getNumberOfAgents() returns {0}", context.getNumberOfAgents())

        'call getAllAgentsOverlapped
        '(this shows the linkage for an array out argument, and also gives as another handle to compare to placedAgent)

        Dim overlapped As Agent()
        'seems like you just have to ignore the warning about overlapped being uninitialised here
        '(the parameter is declared as an out parameter in the CLR interop, and it is ok for overlapped to be uninitialised, but out parameters are not supported in Visual Basic)
        mesh.getAllAgentsOverlapped(shape, context, pos, overlapped)
        Console.WriteLine("number of agents overlapped = {0}", overlapped.Length)

        'we can't compare interface object handles directly, but a getComparisonToken() method is supplied for this purpose
        '(and can also be used for sorting, if this is required)

        Console.WriteLine("placedAgent.getComparisonToken() returns {0}", placedAgent.getComparisonToken())
        For Each overlappedAgent As Agent In overlapped
            Console.WriteLine("overlappedAgent.getComparisonToken() returns {0}", overlappedAgent.getComparisonToken())
        Next

        'a simple file output stream callback is supplied

        Dim os As OutputStream = New FileOutputStream("savedMesh.xml")
        mesh.saveGround("xml", False, os)

        'object lifetime notes

        Dim mesh2 As Mesh = pathEngine.buildMeshFromContent(contentMeshes, noOptions)
        mesh2 = Nothing
        '** PathEngine interface objects are not garbage collected, so mesh2 will not be cleaned up automatically
        '** and will therefore now be leaked, unless pathEngine.deleteAllObjects() is called

        mesh.destroy() 'clean up this interface object explicitly
        '** but don't forget that the interface object is now no longer valid, and methods should not be called on this object
        '** and note that dependant objects such as placed agents and collision contexts are also destroyed with the mesh

        pathEngine.deleteAllObjects() 'cleans up all objects created by PathEngine

        Console.WriteLine("(completed, press enter)")
        Console.ReadLine()
    End Sub

End Module
