#include "GameInfo.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

using std::min;
using std::max;

bool LevelInfo::findRandomFreePositionInBox(Vector2& result, const Vector2& minPos, const Vector2& maxPos)
{
//  Find a random position for a character to move to in an area.
//  None is returned if no position could be found.
    Vector2 area[] = {minPos, maxPos};
    float minX = min(max(0.0f, area[0].x), float(width-1)), minY = min(max(0.0f, area[0].y), float(height-1));
    float maxX = min(max(0.0f, area[1].x), float(width-1)), maxY = min(max(0.0f, area[1].y), float(height-1));
    float rangeX = maxX - minX, rangeY = maxY - minY;

    if ((rangeX == 0.0) || (rangeY == 0.0))
        return false;

    for(int i=0; i<100; ++i)
    {
        float x = (float)rand()/RAND_MAX * rangeX + minX;
        float y = (float)rand()/RAND_MAX * rangeY + minY;
        int ix = (int)x, iy = (int)y;

        float* blocks = blockHeights.get();

        // check if there are any blocks under current position
        if (blocks[ix+iy*width] > 0)
            continue;

        // check if there are any blocks in the four cardinal directions
        if ((x - ix) < characterRadius && ix > 0 && blocks[ix-1+iy*width] > 0)
            continue;
        if ((ix + 1 - x) < characterRadius && ix < width - 1 && blocks[ix+1+iy*width] > 0)
            continue;
        if ((y - iy) < characterRadius && iy > 0 && blocks[ix+(iy-1)*width] > 0)
            continue;
        if ((iy + 1 - y) < characterRadius && iy < height - 1 && blocks[ix+(iy+1)*width] > 0)
            continue;

        // check if there are any blocks in the four diagonals
        if ((x - ix) < characterRadius && (y - iy) < characterRadius && ix > 0 && iy > 0 && blocks[ix-1+(iy-1)*width] > 0)
            continue;
        if ((ix + 1 - x) < characterRadius && (y - iy) < characterRadius && ix < width - 1 && iy > 0 && blocks[ix+1+(iy-1)*width] > 0)
            continue;
        if ((x - ix) < characterRadius && (iy + 1 - y) < characterRadius && ix > 0 && iy < height - 1 && blocks[ix-1+(iy+1)*width] > 0)
            continue;
        if ((x + 1 - ix) < characterRadius && (iy + 1 - y) < characterRadius && ix < width - 1 && iy < height - 1 && blocks[ix+1+(iy+1)*width] > 0)
            continue;

        result = Vector2(x,y);
        return true;
    }
    return false;
}

// Find a free position near 'target' for a character to move to.
bool LevelInfo::findNearestFreePosition( Vector2& result, const Vector2& target )
{
    for(int i=0; i<10; ++i)
    {
        Vector2 areaMin = Vector2(target.x - i, target.y - i);
        Vector2 areaMax = Vector2(target.x + i, target.y + i);
        
        if(findRandomFreePositionInBox(result, areaMin, areaMax))
            return true;
    }
    return false;
}
