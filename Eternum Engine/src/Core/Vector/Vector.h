/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: Vector
* Description:
*     A fixed-size mathematical vector that supports any dimension N > 0 and
*     common vector operations (add/sub, scalar ops, dot, length, normalize).
*
* Author:     Jax Clayton
* Created:    8/8/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef VECTOR_H
#define VECTOR_H


/// @brief Fixed-size vector with N components of type T.
template <typename T, std::size_t N>
class Vector
{
    static_assert(N > 0, "Vector<N>: N must be > 0");

public:
    using value_type = T;
    static constexpr std::size_t kSize = N;

    //-----------------------------------------------------------------------------
    // Constructors
    //-----------------------------------------------------------------------------
    Vector()
    {
        if constexpr (std::is_arithmetic_v<T>)
            m_Data.fill(static_cast<T>(0));
        else
            m_Data = {};
    }

    explicit Vector(const T& v)
    {
        m_Data.fill(v);
    }

    Vector(std::initializer_list<T> list)
    {
        if (list.size() != N)
            throw std::invalid_argument("Vector: initializer_list size must equal N");
        std::copy(list.begin(), list.end(), m_Data.begin());
    }

    template <typename It>
    explicit Vector(It first)
    {
        for (std::size_t i = 0; i < N; ++i, ++first)
            m_Data[i] = static_cast<T>(*first);
    }

    //-----------------------------------------------------------------------------
    // Element access (index-based)
    //-----------------------------------------------------------------------------
    T& operator[](std::size_t i) { return m_Data[i]; }
    const T& operator[](std::size_t i) const { return m_Data[i]; }

    T& at(std::size_t i)
    {
        if (i >= N) throw std::out_of_range("Vector::at out of range");
        return m_Data[i];
    }
    const T& at(std::size_t i) const
    {
        if (i >= N) throw std::out_of_range("Vector::at out of range");
        return m_Data[i];
    }

    //-----------------------------------------------------------------------------
    // Named accessors
    //-----------------------------------------------------------------------------
    T& x() { static_assert(N >= 1, "No x in this vector"); return m_Data[0]; }
    T& y() { static_assert(N >= 2, "No y in this vector"); return m_Data[1]; }
    T& z() { static_assert(N >= 3, "No z in this vector"); return m_Data[2]; }
    T& w() { static_assert(N >= 4, "No w in this vector"); return m_Data[3]; }

    const T& x() const { static_assert(N >= 1, "No x in this vector"); return m_Data[0]; }
    const T& y() const { static_assert(N >= 2, "No y in this vector"); return m_Data[1]; }
    const T& z() const { static_assert(N >= 3, "No z in this vector"); return m_Data[2]; }
    const T& w() const { static_assert(N >= 4, "No w in this vector"); return m_Data[3]; }

    //-----------------------------------------------------------------------------
    // Iteration
    //-----------------------------------------------------------------------------
    auto begin() noexcept { return m_Data.begin(); }
    auto end() noexcept { return m_Data.end(); }
    auto begin() const noexcept { return m_Data.begin(); }
    auto end() const noexcept { return m_Data.end(); }

    //-----------------------------------------------------------------------------
    // Arithmetic (in-place)
    //-----------------------------------------------------------------------------
    Vector& operator+=(const Vector& rhs)
    {
        for (std::size_t i = 0; i < N; ++i) m_Data[i] += rhs.m_Data[i];
        return *this;
    }
    Vector& operator-=(const Vector& rhs)
    {
        for (std::size_t i = 0; i < N; ++i) m_Data[i] -= rhs.m_Data[i];
        return *this;
    }
    Vector& operator*=(const T& s)
    {
        for (auto& v : m_Data) v *= s;
        return *this;
    }
    Vector& operator/=(const T& s)
    {
        for (auto& v : m_Data) v /= s;
        return *this;
    }

    //-----------------------------------------------------------------------------
    // Unary
    //-----------------------------------------------------------------------------
    Vector operator+() const { return *this; }
    Vector operator-() const
    {
        Vector out;
        for (std::size_t i = 0; i < N; ++i) out[i] = -m_Data[i];
        return out;
    }

    //-----------------------------------------------------------------------------
    // Norms & Products
    //-----------------------------------------------------------------------------
    T Dot(const Vector& rhs) const
    {
        T acc = static_cast<T>(0);
        for (std::size_t i = 0; i < N; ++i) acc += m_Data[i] * rhs.m_Data[i];
        return acc;
    }

    T LengthSq() const { return this->Dot(*this); }

    auto Length() const
    {
        using std::sqrt;
        if constexpr (std::is_floating_point_v<T>)
            return sqrt(LengthSq());
        else
            return std::sqrt(static_cast<long double>(LengthSq()));
    }

    Vector Normalized() const
    {
        const auto len = Length();
        if (len == static_cast<decltype(len)>(0)) return *this;
        Vector out(*this);
        if constexpr (std::is_floating_point_v<T>)
            out /= static_cast<T>(len);
        else
        {
            for (std::size_t i = 0; i < N; ++i)
                out[i] = static_cast<T>(static_cast<long double>(out[i]) / static_cast<long double>(len));
        }
        return out;
    }

    void Normalize()
    {
        const auto len = Length();
        if (len == static_cast<decltype(len)>(0)) return;
        if constexpr (std::is_floating_point_v<T>)
            *this /= static_cast<T>(len);
        else
        {
            for (std::size_t i = 0; i < N; ++i)
                m_Data[i] = static_cast<T>(static_cast<long double>(m_Data[i]) / static_cast<long double>(len));
        }
    }

    //-----------------------------------------------------------------------------
    // Comparisons
    //-----------------------------------------------------------------------------
    bool operator==(const Vector& rhs) const
    {
        for (std::size_t i = 0; i < N; ++i)
            if (!(m_Data[i] == rhs.m_Data[i])) return false;
        return true;
    }
    bool operator!=(const Vector& rhs) const { return !(*this == rhs); }

    template <typename U = T>
    std::enable_if_t<std::is_floating_point_v<U>, bool>
    AlmostEqual(const Vector& rhs, U eps = static_cast<U>(1e-6)) const
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            if (std::fabs(static_cast<U>(m_Data[i]) - static_cast<U>(rhs.m_Data[i])) > eps)
                return false;
        }
        return true;
    }

    //-----------------------------------------------------------------------------
    // Named Constructors
    //-----------------------------------------------------------------------------
    static Vector Zero() { return Vector(static_cast<T>(0)); }

    static Vector Unit(std::size_t i)
    {
        Vector v(static_cast<T>(0));
        if (i < N) v[i] = static_cast<T>(1);
        return v;
    }

private:
    std::array<T, N> m_Data;
};

// -----------------------------------------------------------------------------
// Free Operators
// -----------------------------------------------------------------------------
template <typename T, std::size_t N>
inline Vector<T, N> operator+(Vector<T, N> lhs, const Vector<T, N>& rhs) { lhs += rhs; return lhs; }
template <typename T, std::size_t N>
inline Vector<T, N> operator-(Vector<T, N> lhs, const Vector<T, N>& rhs) { lhs -= rhs; return lhs; }
template <typename T, std::size_t N>
inline Vector<T, N> operator*(Vector<T, N> v, const T& s) { v *= s; return v; }
template <typename T, std::size_t N>
inline Vector<T, N> operator*(const T& s, Vector<T, N> v) { v *= s; return v; }
template <typename T, std::size_t N>
inline Vector<T, N> operator/(Vector<T, N> v, const T& s) { v /= s; return v; }

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
template <typename T, std::size_t N>
inline T Dot(const Vector<T, N>& a, const Vector<T, N>& b) { return a.Dot(b); }
template <typename T, std::size_t N>
inline auto Length(const Vector<T, N>& a) { return a.Length(); }
template <typename T, std::size_t N>
inline Vector<T, N> Normalize(const Vector<T, N>& a) { return a.Normalized(); }

// -----------------------------------------------------------------------------
// Stream Output
// -----------------------------------------------------------------------------
template <typename T, std::size_t N>
inline std::ostream& operator<<(std::ostream& os, const Vector<T, N>& v)
{
    os << "Vector" << N << "(";
    for (std::size_t i = 0; i < N; ++i)
    {
        os << v[i];
        if (i + 1 < N) os << ", ";
    }
    os << ")";
    return os;
}


// -----------------------------------------------------------------------------
// Common Aliases
// -----------------------------------------------------------------------------
using Vec2f = Vector<float, 2>;
using Vec3f = Vector<float, 3>;
using Vec4f = Vector<float, 4>;

using Vec2d = Vector<double, 2>;
using Vec3d = Vector<double, 3>;
using Vec4d = Vector<double, 4>;

using Vec2i = Vector<int, 2>;
using Vec3i = Vector<int, 3>;
using Vec4i = Vector<int, 4>;

#endif // VECTOR_H
