/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: VectorTests
* Description:
*      Tests for the templated Vector<T, N> class: construction, named accessors,
*      arithmetic, dot/length/normalize, comparisons, and helpers.
*
* Author:     Jax Clayton
* Created:    8/8/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <pch.h>
// ---------------------------------------------------------
// Construction
// ---------------------------------------------------------
TEST(VectorTests, DefaultConstructZerosFloat)
{
    Vector<float, 3> v;
    EXPECT_FLOAT_EQ(v[0], 0.0f);
    EXPECT_FLOAT_EQ(v[1], 0.0f);
    EXPECT_FLOAT_EQ(v[2], 0.0f);
}

TEST(VectorTests, FillConstructor)
{
    Vector<int, 5> v(7);
    for (std::size_t i = 0; i < 5; ++i)
        EXPECT_EQ(v[i], 7);
}

TEST(VectorTests, InitializerListExactSize)
{
    Vector<double, 4> v{1.0, 2.0, 3.0, 4.0};
    EXPECT_DOUBLE_EQ(v[0], 1.0);
    EXPECT_DOUBLE_EQ(v[1], 2.0);
    EXPECT_DOUBLE_EQ(v[2], 3.0);
    EXPECT_DOUBLE_EQ(v[3], 4.0);
}

TEST(VectorTests, InitializerListThrowsOnWrongSize)
{
    EXPECT_THROW((Vector<int, 3>{1, 2}), std::invalid_argument);
    EXPECT_THROW((Vector<int, 3>{1, 2, 3, 4}), std::invalid_argument);
}

// ---------------------------------------------------------
// Named Accessors
// ---------------------------------------------------------
TEST(VectorTests, NamedAccessorsXYZWPresentWhenDimensionAllows)
{
    Vector<float, 2> v2{1.0f, 2.0f};
    EXPECT_FLOAT_EQ(v2.x(), 1.0f);
    EXPECT_FLOAT_EQ(v2.y(), 2.0f);

    Vector<float, 3> v3{1.0f, 2.0f, 3.0f};
    EXPECT_FLOAT_EQ(v3.x(), 1.0f);
    EXPECT_FLOAT_EQ(v3.y(), 2.0f);
    EXPECT_FLOAT_EQ(v3.z(), 3.0f);

    Vector<float, 4> v4{1.0f, 2.0f, 3.0f, 4.0f};
    EXPECT_FLOAT_EQ(v4.x(), 1.0f);
    EXPECT_FLOAT_EQ(v4.y(), 2.0f);
    EXPECT_FLOAT_EQ(v4.z(), 3.0f);
    EXPECT_FLOAT_EQ(v4.w(), 4.0f);
}

TEST(VectorTests, NamedAccessorsAreWritable)
{
    Vector<float, 3> v{0.f, 0.f, 0.f};
    v.x() = 4.f; v.y() = 5.f; v.z() = 6.f;
    EXPECT_FLOAT_EQ(v[0], 4.f);
    EXPECT_FLOAT_EQ(v[1], 5.f);
    EXPECT_FLOAT_EQ(v[2], 6.f);
}

// ---------------------------------------------------------
// Arithmetic Operators
// ---------------------------------------------------------
TEST(VectorTests, PlusMinusMulDivInPlace)
{
    Vector<int, 3> a{1, 2, 3};
    Vector<int, 3> b{4, 5, 6};

    a += b; // a = {5,7,9}
    EXPECT_EQ(a[0], 5); EXPECT_EQ(a[1], 7); EXPECT_EQ(a[2], 9);

    a -= Vector<int, 3>{1, 1, 1}; // {4,6,8}
    EXPECT_EQ(a[0], 4); EXPECT_EQ(a[1], 6); EXPECT_EQ(a[2], 8);

    a *= 2; // {8,12,16}
    EXPECT_EQ(a[0], 8); EXPECT_EQ(a[1], 12); EXPECT_EQ(a[2], 16);

    a /= 4; // {2,3,4}
    EXPECT_EQ(a[0], 2); EXPECT_EQ(a[1], 3); EXPECT_EQ(a[2], 4);
}

TEST(VectorTests, FreeOperatorsReturnNew)
{
    Vector<float, 2> a{1.f, 2.f};
    Vector<float, 2> b{3.f, 4.f};

    auto c = a + b; // {4,6}
    EXPECT_FLOAT_EQ(c[0], 4.f); EXPECT_FLOAT_EQ(c[1], 6.f);

    auto d = c - a; // {3,4}
    EXPECT_FLOAT_EQ(d[0], 3.f); EXPECT_FLOAT_EQ(d[1], 4.f);

    auto e = d * 0.5f; // {1.5,2}
    EXPECT_FLOAT_EQ(e[0], 1.5f); EXPECT_FLOAT_EQ(e[1], 2.f);

    auto f = 2.0f * e; // {3,4}
    EXPECT_FLOAT_EQ(f[0], 3.f); EXPECT_FLOAT_EQ(f[1], 4.f);

    auto g = f / 2.0f; // {1.5,2}
    EXPECT_FLOAT_EQ(g[0], 1.5f); EXPECT_FLOAT_EQ(g[1], 2.f);
}

TEST(VectorTests, UnaryPlusMinus)
{
    Vector<int, 3> a{1, -2, 3};
    auto b = +a;
    auto c = -a;
    EXPECT_EQ(b[0], 1); EXPECT_EQ(b[1], -2); EXPECT_EQ(b[2], 3);
    EXPECT_EQ(c[0], -1); EXPECT_EQ(c[1], 2); EXPECT_EQ(c[2], -3);
}

// ---------------------------------------------------------
// Dot / Length / Normalize
// ---------------------------------------------------------
TEST(VectorTests, DotAndLengthFloat)
{
    Vector<float, 3> a{1.f, 2.f, 2.f};
    EXPECT_FLOAT_EQ(a.Dot(a), 9.f);
    EXPECT_FLOAT_EQ(Length(a), 3.f);
    EXPECT_FLOAT_EQ(a.Length(), 3.f);
}

TEST(VectorTests, NormalizeFloat)
{
    Vector<float, 3> a{0.f, 3.f, 4.f};
    auto n = a.Normalized();
    EXPECT_NEAR(n[0], 0.f, 1e-6f);
    EXPECT_NEAR(n[1], 0.6f, 1e-6f);
    EXPECT_NEAR(n[2], 0.8f, 1e-6f);

    a.Normalize();
    EXPECT_NEAR(a[0], 0.f, 1e-6f);
    EXPECT_NEAR(a[1], 0.6f, 1e-6f);
    EXPECT_NEAR(a[2], 0.8f, 1e-6f);
}

TEST(VectorTests, LengthIntPromotesForSqrt)
{
    Vector<int, 2> a{3, 4};
    // Length() returns promoted floating result for ints; compare as double.
    EXPECT_DOUBLE_EQ(static_cast<double>(a.Length()), 5.0);
}

// ---------------------------------------------------------
// Comparisons
// ---------------------------------------------------------
TEST(VectorTests, EqualityAndInequality)
{
    Vector<int, 4> a{1,2,3,4};
    Vector<int, 4> b{1,2,3,4};
    Vector<int, 4> c{1,2,3,5};

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST(VectorTests, AlmostEqualForFloat)
{
    Vector<float, 3> a{1.0f, 2.0f, 3.0f};
    Vector<float, 3> b{1.0f + 1e-7f, 2.0f - 5e-7f, 3.0f};
    EXPECT_TRUE(a.AlmostEqual(b, 1e-6f));
    EXPECT_FALSE(a.AlmostEqual(b, 1e-9f));
}

// ---------------------------------------------------------
// Helpers and Named Constructors
// ---------------------------------------------------------
TEST(VectorTests, ZeroAndUnit)
{
    auto z = Vector<int, 3>::Zero();
    EXPECT_EQ(z[0], 0); EXPECT_EQ(z[1], 0); EXPECT_EQ(z[2], 0);

    auto u1 = Vector<int, 3>::Unit(1);
    EXPECT_EQ(u1[0], 0); EXPECT_EQ(u1[1], 1); EXPECT_EQ(u1[2], 0);

    // Out-of-range index should just yield zero vector according to implementation
    auto u5 = Vector<int, 3>::Unit(5);
    EXPECT_EQ(u5[0], 0); EXPECT_EQ(u5[1], 0); EXPECT_EQ(u5[2], 0);
}

// ---------------------------------------------------------
// Iteration
// ---------------------------------------------------------
TEST(VectorTests, RangeForIteration)
{
    Vector<int, 3> v{1, 2, 3};
    int sum = 0;
    for (int x : v) sum += x;
    EXPECT_EQ(sum, 6);
}
