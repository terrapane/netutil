/*
 *  variable_integer.h
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file defines variable-width integer types.  These types are
 *      either signed or unsigned integer values, the definition of which
 *      exists just to facilitate disambiguation of types when writing to or
 *      reading from the DataBuffer object.
 *
 *  Portability Issues:
 *      None.
 */

#pragma once

#include <cstdint>
#include <concepts>
#include <type_traits>

namespace Terra::NetUtil
{

template<typename T,
         typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
class VariableInteger
{
    public:
        using value_type = T;

        VariableInteger() : value{} {}
        VariableInteger(T value) : value{value} {}
        VariableInteger(VariableInteger<T> &other) { value = other.value; }
        VariableInteger(VariableInteger<T> &&other) { value = other.value; }
        ~VariableInteger() = default;

        operator T() const { return value; }
        explicit operator bool() const { return value != 0; }
        bool operator!() const { return value == 0; }
        constexpr VariableInteger<T> &operator=(const T &other)
        {
            value = other;
            return *this;
        }
        constexpr VariableInteger<T> &operator=(const VariableInteger<T> &other)
        {
            value = other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator+=(
                                            const VariableInteger<T> &other)
        {
            value += other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator-=(
                                            const VariableInteger<T> &other)
        {
            value -= other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator*=(
                                            const VariableInteger<T> &other)
        {
            value *= other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator/=(
                                            const VariableInteger<T> &other)
        {
            value /= other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator<<=(
                                            const VariableInteger<T> &other)
        {
            value <<= other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator>>=(
                                            const VariableInteger<T> &other)
        {
            value >>= other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator|=(
                                            const VariableInteger<T> &other)
        {
            value |= other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator&=(
                                            const VariableInteger<T> &other)
        {
            value &= other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator^=(
                                            const VariableInteger<T> &other)
        {
            value ^= other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator%=(
                                            const VariableInteger<T> &other)
        {
            value %= other.value;
            return *this;
        }
        constexpr VariableInteger<T> &operator++()
        {
            value++;
            return *this;
        }
        constexpr VariableInteger<T> operator++(int)
        {
            VariableInteger<T> old_value = *this;
            value++;
            return old_value;
        }
        constexpr VariableInteger<T> &operator--()
        {
            value--;
            return *this;
        }
        constexpr VariableInteger<T> operator--(int)
        {
            VariableInteger<T> old_value = *this;
            value--;
            return old_value;
        }

    protected:
        T value;
};

// Define concrete types for use in applications
using VarUint64_t = VariableInteger<std::uint64_t>;
using VarInt64_t = VariableInteger<std::int64_t>;
using VarUint32_t = VariableInteger<std::uint32_t>;
using VarInt32_t = VariableInteger<std::int32_t>;
using VarUint16_t = VariableInteger<std::uint16_t>;
using VarInt16_t = VariableInteger<std::int16_t>;

// Define concepts used with DataBuffer template functions
template<typename T>
concept VariableUnsignedInteger =
    std::same_as<T, VariableInteger<typename T::value_type>> &&
    std::unsigned_integral<typename T::value_type>;

template<typename T>
concept VariableSignedInteger =
    std::same_as<T, VariableInteger<typename T::value_type>> &&
    std::signed_integral<typename T::value_type>;

} // namespace Terra::NetUtil
