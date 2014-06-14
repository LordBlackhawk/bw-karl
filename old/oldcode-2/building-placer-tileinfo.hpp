#pragma once

// Numbering of building side ('d' used in TileInformation::space, ...):
//          1
//       -------
//      |       |
//    2 |       | 0
//      |       |
//       -------
//          3

namespace
{
    int spaceHelper(bool b1, bool b2, bool b3, bool b4)
    {
        if (!b1)
            return 0;
        if (!b2)
            return 8;
        if (!b3)
            return 16;
        if (!b4)
            return 24;
        return 32;
    }
}

TileInformation::TileInformation()
    : buildable(false), pre(NULL), building(NULL), energytime(Precondition::Impossible), energyobj(NULL),
      creeptime(Precondition::Impossible), creepobj(NULL)
{
    for (int k=0; k<4; ++k)
        for (int l=0; l<4; ++l)
            subtiles[k][l] = false;
}

int TileInformation::space(int d)
{
    bool px[4] = { subtiles[0][0] && subtiles[0][1] && subtiles[0][2] && subtiles[0][3],
                   subtiles[1][0] && subtiles[1][1] && subtiles[1][2] && subtiles[1][3],
                   subtiles[2][0] && subtiles[2][1] && subtiles[2][2] && subtiles[2][3],
                   subtiles[3][0] && subtiles[3][1] && subtiles[3][2] && subtiles[3][3] };

    bool py[4] = { subtiles[0][0] && subtiles[1][0] && subtiles[2][0] && subtiles[3][0],
                   subtiles[0][1] && subtiles[1][1] && subtiles[2][1] && subtiles[3][1],
                   subtiles[0][2] && subtiles[1][2] && subtiles[2][2] && subtiles[3][2],
                   subtiles[0][3] && subtiles[1][3] && subtiles[2][3] && subtiles[3][3] };

    switch (d)
    {
        case 0: return spaceHelper(px[3], px[2], px[1], px[0]);
        case 1: return spaceHelper(py[0], py[1], py[2], py[3]);
        case 2: return spaceHelper(px[0], px[1], px[2], px[3]);
        case 3: return spaceHelper(py[3], py[2], py[1], py[0]);
        default:
            return -1;
    }
}
