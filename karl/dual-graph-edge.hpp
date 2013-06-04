DualEdge* edgeByIndex(bool hor, int x, int y)
{
    if (hor) {
        return &(baseEdge[x + y * (xsize-1)]);
    } else {
        return &(baseEdge[x + y * xsize + ysize * (xsize-1)]);
    }
}

bool indexByEdge(const DualEdge* edge, bool& hor, int& x, int& y)
{
    int diff = edge - baseEdge;
    hor = (diff < ysize * (xsize - 1));
    if (hor) {
        x = diff % (xsize - 1);
        y = diff / (xsize - 1);
        return (diff >= 0);
    } else {
        diff -= ysize * (xsize - 1);
        x = diff % xsize;
        y = diff / xsize;
        return (diff >= 0) && (y < xsize);
    }
}

DualEdge* projectToGraph(const BWAPI::TilePosition& pos, Direction::Type dir)
{
    switch (dir)
    {
        case Direction::O: return edgeByIndex(false, pos.x()+1, pos.y());
        case Direction::N: return edgeByIndex(true,  pos.x(), pos.y());
        case Direction::W: return edgeByIndex(false, pos.x(), pos.y());
        case Direction::S: return edgeByIndex(true,  pos.x(), pos.y()+1);
        default:
            assert(false);
            return NULL;
    }
}

DualNode* DualEdge::getBeginNode() const
{
    bool hor; int x, y;
    assert( indexByEdge(this, hor, x, y) );
    return nodeByIndex(x, y);
}

DualNode* DualEdge::getEndNode() const
{
    bool hor; int x, y;
    assert( indexByEdge(this, hor, x, y) );
    if (hor) {
        return nodeByIndex(x+1, y);
    } else {
        return nodeByIndex(x, y+1);
    }
}

BWAPI::TilePosition DualEdge::getLeftTile() const
{
    bool hor; int x, y;
    assert( indexByEdge(this, hor, x, y) );
    if (hor) {
        return TilePosition(x, y-1);
    } else {
        return TilePosition(x, y);
    }
}

BWAPI::TilePosition DualEdge::getRightTile() const
{
    bool hor; int x, y;
    assert( indexByEdge(this, hor, x, y) );
    if (hor) {
        return TilePosition(x, y);
    } else {
        return TilePosition(x-1, y);
    }
}

BWAPI::Position DualEdge::getPosition() const
{
    Position b = getBeginNode()->getPosition();
    Position e = getEndNode()->getPosition();
    return Position((b.x() + e.x())/2, (b.y() + e.y())/2);
}
