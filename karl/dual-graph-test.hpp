#define TEST(x) assert(x)

void testDualGraph()
{
    LOG << "Dual graph tests starting...";

    baseNode = NULL;
    baseEdge = NULL;
    xsize    = 10;
    ysize    = 15;

    DualNode* zero   = projectToGraph(Position(0, 0));
    DualNode* oneone = projectToGraph(Position(32, 32));
    DualNode* twotwo = projectToGraph(Position(64, 64));

    // Test projectToGraph.
    TEST( projectToGraph(Position(15, 15)) == projectToGraph(Position(13, -16)) );
    TEST( projectToGraph(Position(15, 15)) != projectToGraph(Position(13, 16)) );

    TEST( projectToGraph(TilePosition(1, 1), Direction::O) == twotwo->getEdge(Direction::N) );
    TEST( projectToGraph(TilePosition(1, 1), Direction::N) == oneone->getEdge(Direction::O) );
    TEST( projectToGraph(TilePosition(1, 1), Direction::W) == oneone->getEdge(Direction::S) );
    TEST( projectToGraph(TilePosition(1, 1), Direction::S) == twotwo->getEdge(Direction::W) );
    
    // Test hasEdge.
    TEST( oneone->hasEdge(Direction::O) );
    TEST( oneone->hasEdge(Direction::N) );
    TEST( oneone->hasEdge(Direction::W) );
    TEST( oneone->hasEdge(Direction::S) );
    TEST( zero->hasEdge(Direction::O) );
    TEST( !zero->hasEdge(Direction::N) );
    TEST( !zero->hasEdge(Direction::W) );
    TEST( zero->hasEdge(Direction::S) );

    // Test getPosition.
    TEST( oneone->getPosition() == Position(32, 32) );

    // Test consistency getEdge/getStartNode/getEndNode.
    TEST( twotwo->getEdge(Direction::O)->getBeginNode() == twotwo );
    TEST( twotwo->getEdge(Direction::N)->getEndNode()   == twotwo );
    TEST( twotwo->getEdge(Direction::W)->getEndNode()   == twotwo );
    TEST( twotwo->getEdge(Direction::S)->getBeginNode() == twotwo );
    
    // Test getLeftTile/getRightTile.
    TEST( twotwo->getEdge(Direction::O)->getLeftTile()  == TilePosition(2, 1) );
    TEST( twotwo->getEdge(Direction::O)->getRightTile() == TilePosition(2, 2) );
    TEST( twotwo->getEdge(Direction::N)->getLeftTile()  == TilePosition(2, 1) );
    TEST( twotwo->getEdge(Direction::N)->getRightTile() == TilePosition(1, 1) );

    TEST( projectToGraph(TilePosition(1, 1), Direction::O)->getRightTile()  == TilePosition(1, 1) );
    TEST( projectToGraph(TilePosition(1, 1), Direction::N)->getRightTile() == TilePosition(1, 1) );
    TEST( projectToGraph(TilePosition(1, 1), Direction::W)->getLeftTile()  == TilePosition(1, 1) );
    TEST( projectToGraph(TilePosition(1, 1), Direction::S)->getLeftTile() == TilePosition(1, 1) );

    LOG << "Dual graph tests finsihed.";
}

#undef TEST