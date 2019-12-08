#include <vector>
#include "IntegerPoint2D.h"

void GenerateConvexHull(const std::vector<cIntegerPoint2D>& vecPoints, std::vector<cIntegerPoint2D>& vecResult);
cIntegerPoint2D GetHullCentre(const std::vector<cIntegerPoint2D>& hull);
