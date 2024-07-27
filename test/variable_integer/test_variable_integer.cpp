/*
 *  test_variable_integer.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements unit tests for the VariableInteger object.
 *
 *  Portability Issues:
 *      None.
 */

#include <cstdint>
#include <type_traits>
#include <terra/netutil/variable_integer.h>
#include <terra/stf/stf.h>

using namespace Terra::NetUtil;

// Initialize objects using different construction methods
STF_TEST(TestVariableInteger, Constructors)
{
    [[maybe_unused]] VarUint64_t i;
    [[maybe_unused]] VarUint64_t j1{34};
    [[maybe_unused]] VarUint64_t j2(199);
    [[maybe_unused]] VarUint64_t k = 100;
    [[maybe_unused]] VarUint64_t l{k};
    [[maybe_unused]] VarUint64_t m{std::move(l)};
}

// Test casting to normal integer types
STF_TEST(TestVariableInteger, Casting)
{
    VarUint32_t i = 125;
    std::uint64_t j = 0;
    std::uint64_t k = 0;

    j = static_cast<std::uint32_t>(i);
    STF_ASSERT_EQ(125, j);

    k = i; // implicit conversion
    STF_ASSERT_EQ(125, k);

    VarUint64_t l = 125;
    std::uint32_t m;

    m = l;
    STF_ASSERT_EQ(125, m);

    VarUint32_t n;
    n = i;
    STF_ASSERT_EQ(125, n);
}

// Test boolean operator
STF_TEST(TestVariableInteger, Boolean)
{
    {
        VarUint64_t i = 512;

        if (i)
        {
            STF_ASSERT_TRUE(true);
        }
        else
        {
            // Should not get here
            STF_ASSERT_TRUE(false);
        }

        if (!i)
        {
            // Should not get here
            STF_ASSERT_FALSE(true);
        }
        else
        {
            STF_ASSERT_FALSE(false);
        }
    }

    {
        VarUint64_t i = 0;

        if (i)
        {
            // Should not get here
            STF_ASSERT_TRUE(false);
        }
        else
        {
            STF_ASSERT_TRUE(true);
        }

        if (!i)
        {
            STF_ASSERT_FALSE(false);
        }
        else
        {
            // Should not get here
            STF_ASSERT_FALSE(true);
        }
    }
}

// Test assignment operator
STF_TEST(TestVariableInteger, Assignment)
{
    VarUint64_t i = 1024;
    VarUint64_t j = 512;
    std::uint64_t k;

    STF_ASSERT_NE(i, j);

    j = i;

    STF_ASSERT_EQ(i, j);

    k = i;
    k *= 2;

    j = k;

    STF_ASSERT_EQ(VarUint64_t(2048), j);
    STF_ASSERT_EQ(std::uint64_t(2048), j);
    STF_ASSERT_EQ(2048, j);
}

// Test equality operator
STF_TEST(TestVariableInteger, Equality)
{
    VarUint64_t i = 1024;
    VarUint64_t j = 1024;
    STF_ASSERT_EQ(i, j);
}

// Test various inequality operators
STF_TEST(TestVariableInteger, Inequality)
{
    VarUint64_t i = 1024;
    VarUint64_t j = 2048;
    std::uint64_t k = 1536;

    STF_ASSERT_NE(i, j);
    STF_ASSERT_NE(i, 5);
    STF_ASSERT_NE(5, i);
    STF_ASSERT_NE(k, i);
    STF_ASSERT_NE(i, k);

    STF_ASSERT_LT(i, j);
    STF_ASSERT_LT(i, 2048);
    STF_ASSERT_LT(512, i);
    STF_ASSERT_LT(i, k);
    STF_ASSERT_LT(k, j);

    STF_ASSERT_LE(i, j);
    STF_ASSERT_LE(i, i);
    STF_ASSERT_LE(i, 1024);
    STF_ASSERT_LE(1024, i);
    STF_ASSERT_LE(k, j);
    STF_ASSERT_LE(i, k);

    STF_ASSERT_GT(j, i);
    STF_ASSERT_GT(j, 1024);
    STF_ASSERT_GT(4096, j);
    STF_ASSERT_GT(j, k);
    STF_ASSERT_GT(k, i);

    STF_ASSERT_GE(j, i);
    STF_ASSERT_GE(j, j);
    STF_ASSERT_GE(2048, j);
    STF_ASSERT_GE(j, 2048);
    STF_ASSERT_GE(j, k);
    STF_ASSERT_GE(k, i);
}

// Test addition operators
STF_TEST(TestVariableInteger, Addition)
{
    VarUint32_t i = 1;

    std::uint32_t j = i++;

    STF_ASSERT_EQ(1, j);
    STF_ASSERT_EQ(2, i);

    std::uint32_t k = ++i;
    STF_ASSERT_EQ(3, k);
    STF_ASSERT_EQ(3, i);

    i += 2;
    STF_ASSERT_EQ(5, i);

    i = i + 3;
    STF_ASSERT_EQ(8, i);

    i = 5 + i;
    STF_ASSERT_EQ(13, i);

    VarUint32_t l = 5;

    l = i + l;
    STF_ASSERT_EQ(18, l);

    l = l + i;
    STF_ASSERT_EQ(31, l);
}

// Test subtraction operators
STF_TEST(TestVariableInteger, Subtraction)
{
    VarUint32_t i = 100;

    std::uint32_t j = i--;

    STF_ASSERT_EQ(100, j);
    STF_ASSERT_EQ(99, i);

    std::uint32_t k = --i;
    STF_ASSERT_EQ(98, k);
    STF_ASSERT_EQ(98, i);

    i -= 2;
    STF_ASSERT_EQ(96, i);

    i = i - 3;
    STF_ASSERT_EQ(93, i);

    VarUint32_t l = 3;

    l = i - l;
    STF_ASSERT_EQ(90, l);

    l = 100;
    l = l - i;
    STF_ASSERT_EQ(7, l);

    l = 10 - l;
    STF_ASSERT_EQ(3, l);
}

// Test multiplication operators
STF_TEST(TestVariableInteger, Multiplication)
{
    VarUint64_t i = 5;
    VarUint64_t j = 3;

    VarUint64_t k = i * j;
    STF_ASSERT_EQ(15, k);

    i *= k;
    STF_ASSERT_EQ(75, i);

    j = j * 2;
    STF_ASSERT_EQ(6, j);

    j = 3 * j;
    STF_ASSERT_EQ(18, j);
}

// Test division operators
STF_TEST(TestVariableInteger, Division)
{
    VarUint64_t i = 100;
    VarUint64_t j = 5;

    VarUint64_t k = i / j;
    STF_ASSERT_EQ(20, k);

    i /= k;
    STF_ASSERT_EQ(5, i);

    k = k / 2;
    STF_ASSERT_EQ(10, k);

    k = 1000 / k;
    STF_ASSERT_EQ(100, k);
}

// Test bit shifting operators
STF_TEST(TestVariableInteger, BitShift)
{
    VarUint64_t i = 2;
    VarUint64_t j = 2;

    i <<= 2;
    STF_ASSERT_EQ(8, i);

    i = i << 2;
    STF_ASSERT_EQ(32, i);

    i <<= j;
    STF_ASSERT_EQ(128, i);

    i = i << j;
    STF_ASSERT_EQ(512, i);

    i >>= 2;
    STF_ASSERT_EQ(128, i);

    i = i >> 2;
    STF_ASSERT_EQ(32, i);

    i = i >> j;
    STF_ASSERT_EQ(8, i);
}

// Test bit-wise OR operators
STF_TEST(TestVariableInteger, BitwiseOR)
{
    VarUint64_t i = 1;
    VarUint64_t j = 0b1000;

    i |= 2;
    STF_ASSERT_EQ(3, i);

    i = i | 0b0100;
    STF_ASSERT_EQ(7, i);

    i = i | j;
    STF_ASSERT_EQ(15, i);
}

// Test bit-wise AND operators
STF_TEST(TestVariableInteger, BitwiseAND)
{
    VarUint64_t i = 0b1111;
    VarUint64_t j = 0b1010;

    i &= 0b1110;
    STF_ASSERT_EQ(14, i);

    i = i & 0b0110;
    STF_ASSERT_EQ(6, i);

    i = i & j;
    STF_ASSERT_EQ(2, i);
}

// Test bit-wise NOT operators
STF_TEST(TestVariableInteger, BitwiseNOT)
{
    VarUint64_t i = 0b1111;
    std::uint64_t j = 0b1111;

    i = ~i;
    j = ~j;
    STF_ASSERT_EQ(i, j);
}

// Test bit-wise XOR operators
STF_TEST(TestVariableInteger, BitwiseXOR)
{
    VarUint64_t i = 0b1110;
    std::uint64_t j = 0b1110;

    i ^= 0b0100;
    j ^= 0b0100;
    STF_ASSERT_EQ(i, j);

    i = i ^ 0b1000;
    j = j ^ 0b1000;
    STF_ASSERT_EQ(i, j);
}

// Test modulus operator
STF_TEST(TestVariableInteger, Modulus)
{
    VarUint64_t i = 4;
    std::uint64_t j = 4;

    i %= 3;
    j %= 3;
    STF_ASSERT_EQ(i, j);

    i = 4;
    j = 4;
    STF_ASSERT_EQ(i, j);

    i = i % 3;
    j = i % 3;
    STF_ASSERT_EQ(i, j);
}

// Check that + and - operators do not blow up
STF_TEST(TestVariableInteger, VerifyPlusMinus)
{
    VarInt32_t i = 314;

    STF_ASSERT_EQ(314, +i);
    STF_ASSERT_EQ(-314, -i);
}

void verify_func_call(std::uint64_t)
{
    STF_ASSERT_TRUE(false);
}

void verify_func_call(VarUint64_t)
{
    STF_ASSERT_TRUE(true);
}

// Verify the right function is called
STF_TEST(TestVariableInteger, VerifyFunctionCall)
{
    VarUint64_t i = 5;

    verify_func_call(i);
}
