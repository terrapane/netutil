/*
 *  varint_data_buffer.h
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This object extends the DataBuffer object to support reading and
 *      writing variable width integers.  Since there are various ways one
 *      might implement variable width integer encoding, this particular
 *      implementation is kept separate.  Further, it makes more sense to
 *      keep the core DataBuffer as reasonably simple as possible.
 *
 *      In this implementation of variable-width integer encoding, integers
 *      are encoded with space-efficiency as a focus.  The integer to be
 *      serialized is encoded using 1 to n octets, using the fewest octets
 *      possible to hold the value. On the wire, an integer might be encoded
 *      as follows:
 *
 *          10000011 11111111 01111111
 *          ^        ^        ^--- 0 == final octet
 *
 *      This represents the 16-bit value 0xffff.  This encoder and decoder
 *      logic supports both signed and unsigned variable-width integer types.
 *
 *      WARNING: Due to the fact that VariableInteger types are implemented
 *               to look and act like real normal integer types, using this
 *               class with and trying to write an integer type causes
 *               ambiguity.  The compiler will warn about this issue.  The
 *               solution (as exemplified in this module's test code) is to
 *               call DataBuffer:SetValue(), for example, explicitly, rather
 *               than relying on the compiler to deduce the type.
 *
 *  Portability Issues:
 *      None.
 */

#pragma once

#include "data_buffer.h"
#include "variable_integer.h"

namespace Terra::NetUtil
{

// Define the VarIntDataBuffer object
class VarIntDataBuffer : virtual public DataBuffer
{
    public:
        // Rely on the base class constructors
        using DataBuffer::DataBuffer;

        // Facilitate overload resolution by bringing names into this namespace
        using DataBuffer::SetValue;
        using DataBuffer::GetValue;
        using DataBuffer::AppendValue;
        using DataBuffer::ReadValue;

        virtual ~VarIntDataBuffer() = default;

        std::size_t SetValue(const VarUint64_t &value, std::size_t offset);
        std::size_t SetValue(const VarInt64_t &value, std::size_t offset);
        template<VariableUnsignedInteger T>
        std::size_t SetValue(const T &value, std::size_t offset)
        {
            return SetValue(VarUint64_t(value), offset);
        }
        template<VariableSignedInteger T>
        std::size_t SetValue(const T &value, std::size_t offset)
        {
            return SetValue(VarInt64_t(value), offset);
        }

        std::size_t GetValue(VarUint64_t &value, std::size_t offset) const;
        std::size_t GetValue(VarInt64_t &value, std::size_t offset) const;
        template<VariableUnsignedInteger T>
        std::size_t GetValue(T &value, std::size_t offset) const
        {
            VarUint64_t read_value;
            std::size_t length = GetValue(read_value, offset);
            if (read_value > std::numeric_limits<typename T::value_type>::max())
            {
                throw DataBufferException("Read VarUint exceeds upper limit");
            }
            value = read_value;
            return length;
        }
        template<VariableSignedInteger T>
        std::size_t GetValue(T &value, std::size_t offset) const
        {
            VarInt64_t read_value;
            std::size_t length = GetValue(read_value, offset);
            if (read_value > std::numeric_limits<typename T::value_type>::max())
            {
                throw DataBufferException("Read VarInt exceeds upper limit");
            }
            if (read_value < std::numeric_limits<typename T::value_type>::min())
            {
                throw DataBufferException("Read VarInt exceeds lower limit");
            }
            value = read_value;
            return length;
        }

        std::size_t AppendValue(const VarUint64_t &value);
        std::size_t AppendValue(const VarInt64_t &value);
        template<VariableUnsignedInteger T>
        std::size_t AppendValue(const T &value)
        {
            return AppendValue(VarUint64_t(value));
        }
        template<VariableSignedInteger T>
        std::size_t AppendValue(const T &value)
        {
            return AppendValue(VarInt64_t(value));
        }

        std::size_t ReadValue(VarUint64_t &value);
        std::size_t ReadValue(VarInt64_t &value);
        template<VariableUnsignedInteger T>
        std::size_t ReadValue(T &value)
        {
            VarUint64_t read_value;
            std::size_t length = ReadValue(read_value);
            if (read_value > std::numeric_limits<typename T::value_type>::max())
            {
                throw DataBufferException("Read VarUint exceeds upper limit");
            }
            value = read_value;
            return length;
        }
        template<VariableSignedInteger T>
        std::size_t ReadValue(T &value)
        {
            VarInt64_t read_value;
            std::size_t length = ReadValue(read_value);
            if (read_value > std::numeric_limits<typename T::value_type>::max())
            {
                throw DataBufferException("Read VarInt exceeds upper limit");
            }
            if (read_value < std::numeric_limits<typename T::value_type>::min())
            {
                throw DataBufferException("Read VarInt exceeds lower limit");
            }
            value = read_value;
            return length;
        }

        static std::size_t VarUintSize(const VarUint64_t &value);
        static std::size_t VarIntSize(const VarInt64_t &value);

        // Streaming operators that call function AppendValue / ReadValue;
        // Redefinition is needed, else the compiler will has issues resolving
        // the correct AppendValue() and ReadValue() calls
        template<typename T>
        VarIntDataBuffer &operator<<(const T &value)
        {
            AppendValue(value);
            return *this;
        }
        template<typename T>
        VarIntDataBuffer &operator>>(T &value)
        {
            ReadValue(value);
            return *this;
        }
};

} // namespace Terra::NetUtil
