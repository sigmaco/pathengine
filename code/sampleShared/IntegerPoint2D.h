#ifndef INTEGER_POINT_2D_INCLUDED
#define INTEGER_POINT_2D_INCLUDED

class cIntegerPoint2D
{
    int x, y;

public:

    cIntegerPoint2D()
    {
    }
    cIntegerPoint2D(float x, float y) :
     x(static_cast<int>(x)),
     y(static_cast<int>(y))
    {
    }
    cIntegerPoint2D(int x, int y) :
     x(x),
     y(y)
    {
    }

    int getX() const
    {
        return x;
    }
    int getY() const
    {
        return y;
    }

    bool
    operator==(const cIntegerPoint2D& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }
    bool
    operator!=(const cIntegerPoint2D& rhs) const
    {
        return x != rhs.x || y != rhs.y;
    }

    void
    operator-=(const cIntegerPoint2D& rhs)
    {
        x-=rhs.x;
        y-=rhs.y;
    }
    void
    operator+=(const cIntegerPoint2D& rhs)
    {
        x+=rhs.x;
        y+=rhs.y;
    }

    cIntegerPoint2D
    operator-(const cIntegerPoint2D& rhs) const
    {
        cIntegerPoint2D result(*this);
        result-=rhs;
        return result;
    }
    cIntegerPoint2D
    operator+(const cIntegerPoint2D& rhs) const
    {
        cIntegerPoint2D result(*this);
        result+=rhs;
        return result;
    }

    void rotateRight90()
    {
        int tmp = x;
        x = y;
        y = -tmp;
    }
    void rotateLeft90()
    {
        int tmp = x;
        x = -y;
        y = tmp;
    }
    cIntegerPoint2D left90() const
    {
        cIntegerPoint2D result = *this;
        result.rotateLeft90();
        return result;
    }
    cIntegerPoint2D right90() const
    {
        cIntegerPoint2D result = *this;
        result.rotateRight90();
        return result;
    }

    double dotProduct(const cIntegerPoint2D& rhs) const
    {
        return static_cast<double>(x) * rhs.x + static_cast<double>(y) * rhs.y;
    }

    enum eSide
    {
        SIDE_CENTRE,
        SIDE_LEFT,
        SIDE_RIGHT,
    };

    eSide sideOf(const cIntegerPoint2D& rhs) const;
    int lengthSquared() const;
};

#endif
