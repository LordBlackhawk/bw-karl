DualNode* nodeByIndex(int x, int y)
{
    return &(baseNode[x + y * xsize]);
}

bool indexByNode(const DualNode* node, int& x, int& y)
{
    int diff = node - baseNode;
    x = diff % xsize;
    y = diff / xsize;
    return (diff >= 0) && (y < ysize);
}

BWAPI::Position DualNode::getPosition() const
{
    int x, y;
    assert( indexByNode(this, x, y) );
    return Position(32*x, 32*y);
}

BWTA::Region* DualNode::getRegion() const
{
    return BWTA::getRegion(getPosition());
}

DualNode* projectToGraph(const BWAPI::Position& pos)
{
    int x = (pos.x() + 16) / 32;
    int y = (pos.y() + 16) / 32;
    return nodeByIndex(x, y);
}

bool DualNode::hasEdge(Direction::Type dir) const
{
    int x, y;
    assert( indexByNode(this, x, y) );
    switch (dir)
    {
        case Direction::O: return (x < xsize-1);
        case Direction::N: return (y > 0);
        case Direction::W: return (x > 0);
        case Direction::S: return (y < ysize-1);
        default:
            assert(false);
            return false;
    }
}

DualEdge* DualNode::getEdge(Direction::Type dir) const
{
    int x, y;
    assert( indexByNode(this, x, y) );
    switch (dir)
    {
        case Direction::O: return edgeByIndex(true,  x, y);
        case Direction::N: return edgeByIndex(false, x, y-1);
        case Direction::W: return edgeByIndex(true,  x-1, y);
        case Direction::S: return edgeByIndex(false, x, y);
        default:
            assert(false);
            return NULL;
    }
}
