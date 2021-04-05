#ifndef __VECTOR2D_HPP
#define __VECTOR2D_HPP

template<typename T>
struct Vector2D {
    Vector2D<T> operator-(Vector2D<T> const& other) const
    {
        return Vector2D<T>{this->x - other.x, this->y - other.y};
    }

    Vector2D<T> operator+(Vector2D<T> const& other) const
    {
        return Vector2D<T>{this->x + other.x, this->y + other.y};
    }

    Vector2D<T>& operator+=(Vector2D<T> const& other)
    {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }

    Vector2D<T>& operator-=(Vector2D<T> const& other)
    {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }

    template <typename U>
    Vector2D<T> operator*(U const& other) const
    {
        return Vector2D<T>{other * this->x, other * this->y};
    }

    Vector2D<int> as_int() const
    {
        return Vector2D<int>{int(this->x), int(this->y)};
    }
    
    T x;
    T y;
};

template<typename T>
struct Region2D {
    T x;
    T y;
    T w;
    T h;
};

#endif
