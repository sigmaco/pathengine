Imports PathEngine

Module Module1
    Sub Main()
        Dim pathEngine As PathEngine.PathEngine
        pathEngine = DLLManagement.loadDLL("PathEngine")

        'load a ground mesh

        Dim meshBuffer As Byte() = My.Computer.FileSystem.ReadAllBytes("../resource/meshes/mesh1.xml")
        Dim noOptions As String() = {}
        Dim mesh As Mesh = pathEngine.loadMeshFromBuffer("xml", meshBuffer, noOptions)

        'create a pathfinding agent shape

        Dim coordinateData As Integer() = {-10, -10, -10, 10, 10, 10, 10, -10}
        Dim shape As Shape = pathEngine.newShape(coordinateData)

        ' generate preprocess for this agent shape

        mesh.generateUnobstructedSpaceFor(shape, True, noOptions)
        mesh.generatePathfindPreprocessFor(shape, noOptions)

        'generate random unobstructed positions for query start and end

        'note that:
        'collision contexts provide dynamic state
        'default constructed API object references act like null pointers to API objects in the native mapping
        'null collision context arguments are used in the API to indicate no dynamic state

        Dim nullContext As CollisionContext
        Dim start As Position
        Do
            start = mesh.generateRandomPosition()
        Loop While mesh.testPointCollision(shape, nullContext, start)
        Console.WriteLine("start: {0}:{1},{2}", start.cell, start.x, start.y)
        Dim goal As Position
        Do
            goal = mesh.generateRandomPosition()
        Loop While mesh.testPointCollision(shape, nullContext, goal)
        Console.WriteLine("goal: {0}:{1},{2}", goal.cell, goal.x, goal.y)

        'pathfind!

        Dim path As Path = mesh.findShortestPath(shape, nullContext, start, goal)
        If path.isNull() Then
            Console.WriteLine("No unobstructed path exists between start and goal.")
        Else
            Console.WriteLine("Path found with {0} points:", path.size())
            For i As Integer = 1 To path.size()
                Dim p As Position = path.position(i - 1)
                Console.WriteLine("{0}:{1},{2}", p.cell, p.x, p.y)
            Next
        End If

        Console.WriteLine("(completed, press enter)")
        Console.ReadLine()

    End Sub
End Module
