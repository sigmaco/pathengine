#include "GenerateConvexHull.h"

static bool
PointIsBefore(const cIntegerPoint2D& lhs, const cIntegerPoint2D& test)
{
    if(test.getX() < lhs.getX())
    {
        return true;
    }
    if(test.getX() == lhs.getX())
    {
        return test.getY() < lhs.getY();
    }
    return false;
}

static bool
PointIsToLeftOfOrFurther(const cIntegerPoint2D& base, const cIntegerPoint2D& lhs, const cIntegerPoint2D& test)
{
    cIntegerPoint2D lhsOffset = lhs - base;
    cIntegerPoint2D testOffset = test - base;
    cIntegerPoint2D::eSide side = testOffset.sideOf(lhsOffset);
    if(side == cIntegerPoint2D::SIDE_LEFT)
    {
        return true;
    }
    if(side == cIntegerPoint2D::SIDE_RIGHT)
    {
        return false;
    }
    return testOffset.lengthSquared() > lhsOffset.lengthSquared();
}

static void
GenerateConvexHull(const std::vector<cIntegerPoint2D>& vecPoints, std::vector<cIntegerPoint2D>& vecResult)
{
    //assert(!vecPoints.empty());
    std::vector<bool> pointUsed(vecPoints.size(), false);
    int start = 0;
    int i;
    for(i = 1; i != vecPoints.size(); ++i)
    {
        if(PointIsBefore(vecPoints[start], vecPoints[i]))
        {
            start = i;
        }
	}
	vecResult.push_back(vecPoints[start]);
	while(1)
	{
		int leftMost = -1;
		for(i = 0; i != vecPoints.size(); ++i)
		{
			if(pointUsed[i])
			{
                continue;
            }
            if(vecPoints[i] == vecResult.back())
            {
                continue;
            }
            if(leftMost == -1 || PointIsToLeftOfOrFurther(vecResult.back(), vecPoints[leftMost], vecPoints[i]))
            {
                leftMost = i;
            }
        }
        if(leftMost == -1 || leftMost == start)
        {
            return;
        }
        vecResult.push_back(vecPoints[leftMost]);
        pointUsed[leftMost] = true;
    }
}

static cIntegerPoint2D
GetHullCentre(const std::vector<cIntegerPoint2D>& hull)
{
    //assert(!hull.empty());
    int minX, minY, maxX, maxY;
    minX = maxX = hull.front().getX();
    minY = maxY = hull.front().getY();
    int i;
    for(i = 1; i != hull.size(); ++i)
    {
        if(hull[i].getX() < minX)
        {
            minX = hull[i].getX();
        }
        if(hull[i].getY() < minY)
        {
            minY = hull[i].getY();
        }
        if(hull[i].getX() > maxX)
        {
            maxX = hull[i].getX();
        }
        if(hull[i].getY() > maxY)
        {
            maxY = hull[i].getY();
        }
    }
    maxX -= minX;
    maxX /= 2;
    maxY -= minY;
    maxY /= 2;
    return cIntegerPoint2D(minX + maxX, minY + maxY);
}

