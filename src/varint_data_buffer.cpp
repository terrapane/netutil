/*
 *  varint_data_buffer.cpp
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
 *       This represents the 16-bit value 0xffff.  This encoder and decoder
 *       logic supports both signed and unsigned variable-width integer types.
 *
 *  Portability Issues:
 *      None.
 */

#include <terra/netutil/varint_data_buffer.h>
#include <terra/bitutil/significant_bit.h>

namespace Terra::NetUtil
{

/*
 *  VarIntDataBuffer::SetValue()
 *
 *  Description:
 *      This function will place the given value into the buffer at the
 *      given offset.
 *
 *  Parameters:
 *      value [in]
 *          The value to insert into the data buffer.
 *
 *      offset [in]
 *          The offset into the buffer at which the value will be inserted.
 *
 *  Returns:
 *      The number of octets required by the variable-width unsigned integer.
 *
 *  Comments:
 *      None.
 */
std::size_t VarIntDataBuffer::SetValue(const VarUint64_t &value,
                                       std::size_t offset)
{
    // Determine space requirements for the variable-width integer
    const std::size_t octets_required = VarUintSize(value);

    // Ensure there is sufficient space in the buffer
    if ((offset + octets_required) > buffer_size)
    {
        throw DataBufferException("Attempt to write beyond the buffer");
    }

    // Copy the data to be written
    std::uint64_t data_bits = value;

    // Write octets from right to left (reverse order)
    for (std::size_t i = octets_required; i > 0; i--)
    {
        // Get the group of 7 bits
        std::uint8_t octet = data_bits & 0x7f;

        // Shift the data bits vector by 7 bits
        data_bits >>= 7;

        // If this is not the last octet, set the MSb to 1
        if (i != octets_required) octet |= 0x80;

        // Write the value into the buffer
        buffer[offset + i - 1] = octet;
    }

    return octets_required;
}

/*
 *  VarIntDataBuffer::SetValue()
 *
 *  Description:
 *      This function will place the given value into the buffer at the
 *      given offset.
 *
 *  Parameters:
 *      value [in]
 *          The value to insert into the data buffer.
 *
 *      offset [in]
 *          The offset into the buffer at which the value will be inserted.
 *
 *  Returns:
 *      The number of octets required by the variable-width signed integer.
 *
 *  Comments:
 *      None.
 */
std::size_t VarIntDataBuffer::SetValue(const VarInt64_t &value,
                                       std::size_t offset)
{
    // Determine space requirements for the variable-width integer
    std::size_t octets_required = VarIntSize(value);

    // Ensure there is sufficient space in the buffer
    if ((offset + octets_required) > buffer_size)
    {
        throw DataBufferException("Attempt to write beyond the buffer");
    }

    // Copy the data to be written
    std::int64_t data_bits = value;

    // Write octets from right to left (reverse order)
    for (std::size_t i = octets_required; i > 0; i--)
    {
        // Get the group of 7 bits
        std::uint8_t octet = data_bits & 0x7f;

        // Shift the data bits vector by 7 bits
        data_bits >>= 7;

        // If this is not the last octet, set the MSb to 1
        if (i != octets_required) octet |= 0x80;

        // Write the value into the buffer
        buffer[offset + i - 1] = octet;
    }

    return octets_required;
}

/*
 *  VarIntDataBuffer::GetValue()
 *
 *  Description:
 *      This function will read a value from the buffer at the given offset.
 *
 *  Parameters:
 *      value [out]
 *          The value to retrieve from the buffer.
 *
 *      offset [in]
 *          The offset into the buffer from which the value will be read.
 *
 *  Returns:
 *      The total number of octets the VarUint64_t consumed in the DataBuffer.
 *      The value parameter will be populated with the requested data.  An
 *      exception will be thrown if there is a request to retrieve data outside
 *      of the buffer or if the value appears to be corrupt.
 *
 *  Comments:
 *      While any of the GetValue() functions might throw an exception if
 *      requesting data outside the buffer, careful coders would know when
 *      one is reading outside a buffer.  However, "bad data" might be the
 *      reason an exception might be thrown by this function.
 */
std::size_t VarIntDataBuffer::GetValue(VarUint64_t &value,
                                       std::size_t offset) const
{
    std::uint8_t octet{0x80};
    std::size_t total_octets{0};

    // Initialize the VarUint64_t
    value = 0;

    // Read octets until we find the last one having a 0 MSb
    while ((octet & 0x80) != 0)
    {
        // A 64-bits value should never require more than 10 octets
        if (++total_octets == 11)
        {
            throw DataBufferException("Variable width integer exceeds the "
                                      "maximum supported length");
        }

        // Ensure we do not read beyond the buffer
        if ((offset + total_octets) > buffer_size)
        {
            throw DataBufferException("Attempt to read beyond the data length");
        }

        // Get the target octet
        octet = buffer[offset + total_octets - 1];

        // Add these bits to the returned value
        value = (value << 7) | (octet & 0x7f);
    }

    // If the total length is 10 octets, initial octet must be 0x81
    if ((total_octets == 10) && (buffer[offset] != 0x81))
    {
        throw DataBufferException("Variable width integer read from the buffer "
                                  "is malformed");
    }

    return total_octets;
}

/*
 *  VarIntDataBuffer::GetValue()
 *
 *  Description:
 *      This function will read a value from the buffer at the given offset.
 *
 *  Parameters:
 *      value [out]
 *          The value to retrieve from the buffer.
 *
 *      offset [in]
 *          The offset into the buffer from which the value will be read.
 *
 *  Returns:
 *      The total number of octets the VarInt consumed in the DataBuffer.  The
 *      value parameter will be populated with the requested data.  An exception
 *      will be thrown if there is a request to retrieve data outside of the
 *      buffer or if the value appears to be corrupt.
 *
 *  Comments:
 *      While any of the GetValue() functions might throw an exception if
 *      requesting data outside the buffer, careful coders would know when
 *      one is reading outside a buffer.  However, "bad data" might be the
 *      reason an exception might be thrown by this function.
 */
std::size_t VarIntDataBuffer::GetValue(VarInt64_t &value,
                                       std::size_t offset) const
{
    std::uint8_t octet{0x80};
    std::size_t total_octets{0};

    // Ensure we do not read beyond the buffer
    if (offset > buffer_size)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    // Determine the sign of the number by inspecting the leading sign bit
    value = ((buffer[offset] & 0x40) != 0) ? -1 : 0;

    // Read octets until we find the last one having a 0 MSb
    while ((octet & 0x80) != 0)
    {
        // A 64-bits value should never require more than 10 octets
        if (++total_octets == 11)
        {
            throw DataBufferException("VarInt exceeds the maximum supported "
                                      "length");
        }

        // Ensure we do not read beyond the buffer
        if ((offset + total_octets) > buffer_size)
        {
            throw DataBufferException("Attempt to read beyond the data length");
        }

        // Get the target octet
        octet = buffer[offset + total_octets - 1];

        // Add these bits to the returned value
        value = (value << 7) | (octet & 0x7f);
    }

    // If the total length is 10 octets, ensure the initial octet is one
    // of the only two valid values
    if ((total_octets == 10) && (buffer[offset] != 0x80) &&
        (buffer[offset] != 0xff))
    {
        throw DataBufferException("Variable width integer read from the buffer "
                                  "is malformed");
    }

    return total_octets;
}

/*
 *  VarIntDataBuffer::AppendValue()
 *
 *  Description:
 *      This function will append the given value to the end of the existing
 *      data in the buffer as determined by the data length value.
 *
 *  Parameters:
 *      value [in]
 *          The value to append to the end of the existing data.
 *
 *  Returns:
 *      The number of octets required by the variable-width unsigned integer.
 *
 *  Comments:
 *      None.
 */
std::size_t VarIntDataBuffer::AppendValue(const VarUint64_t &value)
{
    std::size_t length = SetValue(value, data_length);
    data_length += length;

    return length;
}

/*
 *  VarIntDataBuffer::AppendValue()
 *
 *  Description:
 *      This function will append the given value to the end of the existing
 *      data in the buffer as determined by the data length value.
 *
 *  Parameters:
 *      value [in]
 *          The value to append to the end of the existing data.
 *
 *  Returns:
 *      The number of octets required by the variable-width signed integer.
 *
 *  Comments:
 *      None.
 */
std::size_t VarIntDataBuffer::AppendValue(const VarInt64_t &value)
{
    std::size_t length = SetValue(value, data_length);
    data_length += length;

    return length;
}

/*
 *  VarIntDataBuffer::ReadValue()
 *
 *  Description:
 *      This function will read a value from the buffer at the current read
 *      position.
 *
 *  Parameters:
 *      value [out]
 *          The value read from the data buffer at the current read position.
 *
 *  Returns:
 *      The number of octets read from the buffer.  The value parameter will be
 *      populated with the requested data.  An exception will be thrown if there
 *      is a request to retrieve data beyond the data length.
 *
 *  Comments:
 *      None.
 */
std::size_t VarIntDataBuffer::ReadValue(VarUint64_t &value)
{
    std::size_t length = GetValue(value, read_position);

    if ((read_position + length) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    read_position += length;

    return length;
}

/*
 *  VarIntDataBuffer::ReadValue()
 *
 *  Description:
 *      This function will read a value from the buffer at the current read
 *      position.
 *
 *  Parameters:
 *      value [out]
 *          The value read from the data buffer at the current read position.
 *
 *  Returns:
 *      The number of octets read from the buffer.  The value parameter will be
 *      populated with the requested data.  An exception will be thrown if there
 *      is a request to retrieve data beyond the data length.
 *
 *  Comments:
 *      None.
 */
std::size_t VarIntDataBuffer::ReadValue(VarInt64_t &value)
{
    std::size_t length = GetValue(value, read_position);

    if ((read_position + length) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    read_position += length;

    return length;
}

/*
 *  VarIntDataBuffer::VarUintSize()
 *
 *  Description:
 *      This function will return the number of octets required to encode
 *      the given variable-width unsigned integer.
 *
 *  Parameters:
 *      value [in]
 *          The value of the variable width unsigned integer.
 *
 *  Returns:
 *      The number of octets required to encode the given variable-width
 *      integer.
 *
 *  Comments:
 *      None.
 */
std::size_t VarIntDataBuffer::VarUintSize(const VarUint64_t &value)
{
    return BitUtil::FindMSb(value) / 7 + 1;
}

/*
 *  VarIntDataBuffer::VarIntSize()
 *
 *  Description:
 *      This function will return the number of octets required to encode
 *      the given variable-width signed integer.
 *
 *  Parameters:
 *      value [in]
 *          The value of the variable width signed integer.
 *
 *  Returns:
 *      The number of octets required to encode the given variable-width
 *      integer.
 *
 *  Comments:
 *      None.
 */
std::size_t VarIntDataBuffer::VarIntSize(const VarInt64_t &value)
{
    return (BitUtil::FindMSb(value) + 1) / 7 + 1;
}

} // namespace Terra::NetUtil
