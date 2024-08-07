/*
 *  data_buffer.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements the DataBuffer object.  This object can be used to
 *      facilitate serializing or deserializing data in network byte order into
 *      and out of a buffer.  The DataBuffer can allocate memory to use
 *      or the user may provide an existing buffer of memory on which
 *      the data buffer will operate.
 *
 *      All of the various types encoded in the DataBuffer are common data
 *      types.  Additional types, including custom types, may be added by
 *      creating derived classes and adding parallel functions.
 *
 *  Portability Issues:
 *      None.
 */

#include <climits>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <terra/netutil/data_buffer.h>
#include <terra/bitutil/byte_order.h>
#include <terra/bitutil/significant_bit.h>

namespace Terra::NetUtil
{

/*
 *  DataBuffer::DataBuffer()
 *
 *  Description:
 *      Constructor for the DataBuffer object that creates an object with no
 *      associated underlying buffer.  One should assign a buffer later via
 *      the SetBuffer() function.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
DataBuffer::DataBuffer() :
    owns_buffer(false),
    buffer(nullptr),
    buffer_size(0),
    data_length(0),
    read_position(0)
{
}

/*
 *  DataBuffer::DataBuffer()
 *
 *  Description:
 *      Constructor for the DataBuffer object that result in a block of
 *      memory being allocated of the specified size.
 *
 *  Parameters:
 *      buffer_size [in]
 *          The size of the buffer the DataBuffer object should allocate.
 *
 *  Returns:
 *      Nothing.  However, an exception of std::bad_alloc may be thrown if
 *      memory allocation fails.
 *
 *  Comments:
 *      None.
 */
DataBuffer::DataBuffer(std::size_t buffer_size) : DataBuffer()
{
    AllocateBuffer(buffer_size);
}

/*
 *  DataBuffer::DataBuffer()
 *
 *  Description:
 *      Constructor for the DataBuffer object.  With this constructor, the
 *      DataBuffer does not take ownership of the given span and will not
 *      attempt to free the associated memory.
 *
 *  Parameters:
 *      buffer [in]
 *          This is a span over which the DataBuffer should operate.
 *          The data length is assumed to be the size of the span.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
DataBuffer::DataBuffer(std::span<std::uint8_t> buffer) : DataBuffer()
{
    SetBuffer(buffer.data(), buffer.size(), buffer.size());
}

/*
 *  DataBuffer::DataBuffer()
 *
 *  Description:
 *      Constructor for the DataBuffer object.  With this constructor, the
 *      DataBuffer does not take ownership of the given buffer and will not
 *      attempt to free the associated memory.
 *
 *  Parameters:
 *      buffer [in]
 *          Pointer to a raw data buffer for the DataBuffer object to use.
 *          If this value is nullptr, it has the effect of clearing the
 *          buffer and the other parameters are ignored.
 *
 *      buffer_size [in]
 *          The length of the given buffer pointer. This value cannot be zero
 *          if a buffer pointer is not nullptr.
 *
 *      data_length [in]
 *          The length of any data in the given buffer.  This value cannot
 *          exceed the buffer size and defaults to zero.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
DataBuffer::DataBuffer(std::uint8_t *buffer,
                       std::size_t buffer_size,
                       std::size_t data_length) : DataBuffer()
{
    SetBuffer(buffer, buffer_size, data_length);
}

/*
 *  DataBuffer::DataBuffer()
 *
 *  Description:
 *      Copy constructor for the DataBuffer object.
 *
 *  Parameters:
 *      other [in]
 *          A reference to the other data buffer to copy.  Regardless of
 *          whether the other DataBuffer owns its buffer or not, this
 *          constructor will allocate its own memory and copy the buffer
 *          contents from the other DataBuffer object.  If the other object
 *          does not have a buffer, then this copy will not, either.
 *
 *  Returns:
 *      Nothing.  However, an exception of std::bad_alloc may be thrown if
 *      memory allocation fails.
 *
 *  Comments:
 *      None.
 */
DataBuffer::DataBuffer(const DataBuffer &other) : DataBuffer()
{
    // Allocate memory and perform a copy only if the other object has a buffer
    if (other.buffer != nullptr)
    {
        // Allocate memory for this DataBuffer object
        AllocateBuffer(other.buffer_size);

        // Copy the octets from the data buffer into the span
        std::copy_n(other.buffer, buffer_size, buffer);

        // Set other internal variables from the other object
        data_length = other.data_length;
        read_position = other.read_position;
    }
}

/*
 *  DataBuffer::DataBuffer()
 *
 *  Description:
 *      Move constructor for the DataBuffer object.
 *
 *  Parameters:
 *      other [in]
 *          A reference to the other data buffer from which to move data.
 *          If the other DataBuffer has no underlying buffer, neither will
 *          this object.
 *
 *  Returns:
 *      Nothing.  However, an exception of std::bad_alloc may be thrown if
 *      memory allocation fails.
 *
 *  Comments:
 *      None.
 */
DataBuffer::DataBuffer(DataBuffer &&other) noexcept : DataBuffer()
{
    // Move data only if the other object has a buffer
    if (other.buffer != nullptr)
    {
        // Assign values from the other object to this object
        owns_buffer = other.owns_buffer;
        buffer = other.buffer;
        buffer_size = other.buffer_size;
        data_length = other.data_length;
        read_position = other.read_position;

        // Clear all values in the other object
        other.owns_buffer = false;
        other.buffer = nullptr;
        other.buffer_size = 0;
        other.data_length = 0;
        other.read_position = 0;
    }
}

/*
 *  DataBuffer::~DataBuffer()
 *
 *  Description:
 *      Destructor for the DataBuffer object.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
DataBuffer::~DataBuffer()
{
    FreeBuffer();
}

/*
 *  DataBuffer::operator=()
 *
 *  Description:
 *      This operator will copy a DataBuffer object to another.  If the size
 *      of this object does not own its underlying buffer or if the underlying
 *      buffer is not the same as the other, a new buffer will be allocated.
 *      The entire data buffer of the other object, regardless of its data
 *      length, is copied into the this object.  If the other object's data
 *      buffer is zero-length underlying buffer, this object will also have a
 *      zero-length buffer.
 *
 *  Parameters:
 *      other [in]
 *          A reference to the other data buffer from which to copy.
 *
 *  Returns:
 *      A reference to the receiving DataBuffer object.  An exception will be
 *      thrown if there is an error allocating memory.
 *
 *  Comments:
 *      None.
 */
DataBuffer &DataBuffer::operator=(const DataBuffer &other)
{
    // If assigning to self, just return this
    if (this == &other) return *this;

    // If this object does not own its buffer or the buffer is not the same
    // size as the other buffer, allocate memory for this DataBuffer
    if (!owns_buffer || (buffer_size != other.buffer_size))
    {
        AllocateBuffer(other.buffer_size);
    }

    // Copy the buffer contents if buffer size is non-zero
    if (other.buffer_size > 0) std::copy_n(other.buffer, buffer_size, buffer);

    // Set other internal variables from the other object
    data_length = other.data_length;
    read_position = other.read_position;

    return *this;
}

/*
 *  DataBuffer::operator=()
 *
 *  Description:
 *      This operator will move the other DataBuffer object to this one.
 *
 *  Parameters:
 *      other [in]
 *          A reference to the other data buffer from which to move.
 *
 *  Returns:
 *      A reference to the receiving DataBuffer object.
 *
 *  Comments:
 *      None.
 */
DataBuffer &DataBuffer::operator=(DataBuffer &&other) noexcept
{
    // Free any previously allocated buffer or clear any set buffer
    FreeBuffer();

    // Move data only if the other object has a buffer
    if (other.buffer != nullptr)
    {
        // Assign values from the other object to this object
        owns_buffer = other.owns_buffer;
        buffer = other.buffer;
        buffer_size = other.buffer_size;
        data_length = other.data_length;
        read_position = other.read_position;

        // Clear all values in the other object
        other.owns_buffer = false;
        other.buffer = nullptr;
        other.buffer_size = 0;
        other.data_length = 0;
        other.read_position = 0;
    }

    return *this;
}

/*
 *  DataBuffer::AllocateBuffer()
 *
 *  Description:
 *      Allocate a memory buffer of the specified size.
 *
 *  Parameters:
 *      buffer_size [in]
 *          The size of the buffer to allocate.
 *
 *  Returns:
 *      Nothing.  However, an exception of std::bad_alloc may be thrown if
 *      memory allocation fails.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AllocateBuffer(std::size_t size)
{
    // Free any previously allocated buffer or clear any set buffer
    FreeBuffer();

    // If the buffer size is zero, then do not allocate any memory
    if (size == 0) return;

    // Attempt to allocate the requested memory
    buffer = new std::uint8_t[size];
    buffer_size = size;
    owns_buffer = true;
}

/*
 *  DataBuffer::FreeBuffer()
 *
 *  Description:
 *      Free any previously allocated memory or, if a buffer was provided,
 *      clear the pointer.  This will also reset the data length and read
 *      position variables.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::FreeBuffer()
{
    // If DataBuffer owns the memory, free it
    if (owns_buffer) delete[] buffer;

    // Reset various buffer-related member variables
    buffer = nullptr;
    owns_buffer = false;
    buffer_size = 0;
    data_length = 0;
    read_position = 0;
}

/*
 *  DataBuffer::GetBufferPointer()
 *
 *  Description:
 *      Get a pointer to the underlying buffer and specified offset.
 *
 *  Parameters:
 *      offset [in]
 *          Offset into the buffer between zero and the size of the buffer.
 *
 *  Returns:
 *      A pointer to the underlying buffer + the offset value.  An exception is
 *      thrown if the requested offset is beyond the size of the underlying
 *      buffer.  If, however, no buffer is assigned, nullptr is returned,
 *      regardless of the offset value.
 *
 *  Comments:
 *      None.
 */
std::uint8_t *DataBuffer::GetBufferPointer(std::size_t offset) const
{
    // If there is no underlying buffer assigned, return nullptr
    if (buffer == nullptr) return nullptr;

    // Ensure the request is not beyond the buffer length
    if (offset >= buffer_size)
    {
        throw DataBufferException("Invalid buffer pointer requested");
    }

    return buffer + offset;
}

/*
 *  DataBuffer::GetBufferPointer()
 *
 *  Description:
 *      Get a span over the data buffer with respect to both the data length
 *      and the read position.
 *
 *  Parameters:
 *      offset [in]
 *          Offset into the buffer between zero and the size of the buffer.
 *
 *  Returns:
 *      A span over the octets in the DataBuffer that take into account the
 *      data_length and read_position variables.
 *
 *  Comments:
 *      None.
 */
std::span<std::uint8_t> DataBuffer::GetBufferSpan() const
{
    return {buffer + read_position, data_length - read_position};
}

/*
 *  DataBuffer::GetBufferSize()
 *
 *  Description:
 *      Returns the size of the underlying buffer.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The size of the underlying buffer.  If there is no buffer, the value
 *      returned will be zero.
 *
 *  Comments:
 *      None.
 */
std::size_t DataBuffer::GetBufferSize() const
{
    return buffer_size;
}

/*
 *  DataBuffer::SetBuffer()
 *
 *  Description:
 *      Instructs DataBuffer to use the specified span.  If the DataBuffer had
 *      previously allocated a buffer, that buffer will be freed first.  The
 *      DataBuffer will not take ownership of the span given and it will not
 *      attempt to free it later.  The read position is also set to the start of
 *      the buffer and the data length will be set to the size of the span.
 *
 *  Parameters:
 *      new_buffer [in]
 *          Span of octets that over which the data buffer will operate.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetBuffer(std::span<std::uint8_t> new_buffer)
{
    SetBuffer(new_buffer.data(), new_buffer.size(), new_buffer.size());
}

/*
 *  DataBuffer::SetBuffer()
 *
 *  Description:
 *      Instructs DataBuffer to use the specified pre-allocated memory.  If the
 *      DataBuffer had previously allocated a buffer, that buffer will be
 *      freed first.  The DataBuffer will not take ownership of the buffer
 *      given and it will not attempt to free it later.  The read position is
 *      also set to the start of the buffer.
 *
 *  Parameters:
 *      new_buffer [in]
 *          Pointer to a raw data buffer for the DataBuffer object to use.
 *          If this value is nullptr, it has the effect of clearing the
 *          buffer and the other parameters are ignored.
 *
 *      new_buffer_size [in]
 *          The length of the given buffer pointer. This value cannot be zero
 *          if a buffer pointer is not nullptr.
 *
 *      new_data_length [in]
 *          The length of any data in the given buffer.  This value cannot
 *          exceed the buffer size.
 *
 *  Returns:
 *      Nothing, though an exception will be thrown if any of the arguments
 *      are invalid.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetBuffer(std::uint8_t *new_buffer,
                           std::size_t new_buffer_size,
                           std::size_t new_data_length)
{
    // Free any existing buffer
    FreeBuffer();

    // Just return if the buffer pointer is nullptr
    if (new_buffer == nullptr) return;

    // Ensure the parameters are sane
    if ((new_buffer_size == 0) || (new_data_length > new_buffer_size))
    {
        throw DataBufferException("Setting buffer with invalid argument(s)");
    }

    // Set various member variables
    buffer = new_buffer;
    buffer_size = new_buffer_size;
    data_length = new_data_length;
}

/*
 *  DataBuffer::GetDataLength()
 *
 *  Description:
 *      Get the length of the data stored in the DataBuffer.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The length of the data stored in the DataBuffer.
 *
 *  Comments:
 *      None.
 */
std::size_t DataBuffer::GetDataLength() const
{
    return data_length;
}

/*
 *  DataBuffer::SetDataLength()
 *
 *  Description:
 *      Set the length of data stored in the DataBuffer.  Setting the data
 *      length will also cause the read position to be reset to the start
 *      of the data buffer.
 *
 *  Parameters:
 *      length [in]
 *          The value to set as the new data length.
 *
 *  Returns:
 *      Nothing is returned, but an exception will be thrown if the given length
 *      is beyond the size of the underlying buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetDataLength(std::size_t length)
{
    if (length > buffer_size)
    {
        throw DataBufferException("Cannot set the data length beyond the "
                                  "buffer size");
    }

    // Set the data_length
    data_length = length;

    // Reset the read position to the start of the buffer
    read_position = 0;
}

/*
 *  DataBuffer::Empty()
 *
 *  Description:
 *      Check to see if the DataBuffer is emtpy or not.  Specifically, the
 *      check is for a non-zero data length.  The data length is updated
 *      either by using the AppendValue() functions, calling SetDataLength(),
 *      or by using the constructor that accepts a data length as an argument.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      True if the DataBuffer is empty, false if not.  Having a zero
 *      size data length indicates an empty DataBuffer.
 *
 *  Comments:
 *      None.
 */
bool DataBuffer::Empty() const
{
    return (data_length == 0);
}

/*
 *  DataBuffer::GetReadPosition()
 *
 *  Description:
 *      Get the current read position in the buffer.  The read position
 *      is the position from which values will be read when ReadValue()
 *      functions are called.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The the current read position in the internal data buffer.
 *
 *  Comments:
 *      The current read position will always have a value between 0 and
 *      the data length.  If the data length is set and the read position
 *      is beyond that value, it will be changed toe be one less than the
 *      data length or zero if the data length is zero.
 */
std::size_t DataBuffer::GetReadPosition() const
{
    return read_position;
}

/*
 *  DataBuffer::SetReadPosition()
 *
 *  Description:
 *      Set the current read position to the specified value.  The value
 *      cannot be equal to or greater than the data length, except for the
 *      case where the value is zero.
 *
 *  Parameters:
 *      position [in]
 *          The new read position value to set.  This must be in the range
 *          of zero to the data length value.
 *
 *  Returns:
 *      Nothing.  However, an exception will be thrown is an attempt is made
 *      to set the read position to an invalid position.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetReadPosition(std::size_t position)
{
    // Ensure the given value is acceptable
    if (position > data_length)
    {
        throw DataBufferException("Attempt to set data buffer read position "
                                  "beyond the data length");
    }

    read_position = position;
}

/*
 *  DataBuffer::AdvanceReadPosition()
 *
 *  Description:
 *      Advance the current read position by the specified distance in octets.
 *
 *  Parameters:
 *      distance [in]
 *          The distance in octets to advance the read position.
 *
 *  Returns:
 *      Nothing.  However, an exception will be thrown is an attempt is made
 *      to advance the read position beyond the actual data in the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AdvanceReadPosition(std::size_t distance)
{
    SetReadPosition(read_position + distance);
}

/*
 *  DataBuffer::GetUnreadLength()
 *
 *  Description:
 *      This function will return the number of octets in the DataBuffer that
 *      have not been read using the ReadValue() functions.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The number of unread octets in the DataBuffer.
 *
 *  Comments:
 *      None.
 */
std::size_t DataBuffer::GetUnreadLength() const
{
    return data_length - read_position;
}

/*
 *  DataBuffer::operator==()
 *
 *  Description:
 *      This operator will compare this data buffer with another.  Two
 *      DataBuffer objects are considered equal if both have the same
 *      data length and the data length portion of the buffer contents are
 *      identical.  The read position value is not a factor in equality.
 *
 *  Parameters:
 *      other [in]
 *          A reference to the other data buffer with which to compare.
 *
 *  Returns:
 *      True if equal, false of not equal.
 *
 *  Comments:
 *      None.
 */
bool DataBuffer::operator==(const DataBuffer &other)
{
    // Is the data length the same?
    if (data_length != other.data_length) return false;

    // If the data length is zero, they are equal
    if (data_length == 0) return true;

    // Compare the buffer contents with respect to the data length
    return std::equal(buffer, buffer + data_length, other.buffer);
}

/*
 *  DataBuffer::operator!=()
 *
 *  Description:
 *      This operator will compare this data buffer with another.  Two
 *      DataBuffer objects are considered equal if both have the same
 *      data length and the data length portion of the buffer contents are
 *      identical.  The read position value is not a factor in equality.
 *
 *  Parameters:
 *      other [in]
 *          A reference to the other data buffer with which to compare.
 *
 *  Returns:
 *      True if not equal, false of equal.
 *
 *  Comments:
 *      None.
 */
bool DataBuffer::operator!=(const DataBuffer &other)
{
    return !(operator==(other));
}

/*
 *  DataBuffer::operator[]()
 *
 *  Description:
 *      This operator will return a reference to the octet in the buffer
 *      at the specified offset.  This operates on the buffer irrespective of
 *      the data length in the buffer.
 *
 *  Parameters:
 *      index [in]
 *          The buffer index for which an octet reference is requested.
 *
 *  Returns:
 *      A reference to the octet at the given index.  If the index is beyond
 *      the data buffer size, an exception will be thrown.
 *
 *  Comments:
 *      None.
 */
std::uint8_t &DataBuffer::operator[](std::size_t index)
{
    if (index >= buffer_size)
    {
        throw DataBufferException("Index is beyond the data buffer");
    }

    return buffer[index];
}

/*
 *  DataBuffer::operator[]()
 *
 *  Description:
 *      This operator will return a reference to the octet in the buffer
 *      at the specified offset.  This operates on the buffer irrespective of
 *      the data length in the buffer.
 *
 *  Parameters:
 *      index [in]
 *          The buffer index for which an octet reference is requested.
 *
 *  Returns:
 *      A reference to the octet at the given index.  If the index is beyond
 *      the data buffer size, an exception will be thrown.
 *
 *  Comments:
 *      None.
 */
const std::uint8_t &DataBuffer::operator[](std::size_t index) const
{
    if (index >= buffer_size)
    {
        throw DataBufferException("Index is beyond the data buffer");
    }

    return buffer[index];
}

/*
 *  DataBuffer::begin()
 *
 *  Description:
 *      This function is used to facilitate passing the DataBuffer object
 *      to functions that utilize iterators.  This allows the DataBuffer to
 *      be passed to functions accepting spans or used in range-based for loops.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      An span iterator for the underlying data buffer.
 *
 *  Comments:
 *      None.
 */
std::span<std::uint8_t>::iterator DataBuffer::begin() const noexcept
{
    return GetBufferSpan().begin();
}

/*
 *  DataBuffer::end()
 *
 *  Description:
 *      This function is used to facilitate passing the DataBuffer object
 *      to functions that utilize iterators.  This allows the DataBuffer to
 *      be passed to functions accepting spans or used in range-based for loops.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      An span iterator for the underlying data buffer.
 *
 *  Comments:
 *      None.
 */
std::span<std::uint8_t>::iterator DataBuffer::end() const noexcept
{
    return GetBufferSpan().end();
}

/*
 *  DataBuffer::SetValue()
 *
 *  Description:
 *      This function will place the given value into the buffer at the
 *      given offset.
 *
 *  Parameters:
 *      value [in]
 *          The span of octets to insert into the data buffer.
 *
 *      offset [in]
 *          The offset into the buffer at which the octets will be inserted.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(const std::span<const std::uint8_t> value,
                          std::size_t offset)
{
    // If there is nothing to write, just return
    if (value.empty()) return;

    // Ensure this operation will not write beyond the buffer
    if ((offset + value.size()) > buffer_size)
    {
        throw DataBufferException("Attempt to write beyond the buffer");
    }

    // Copy the octets from the data buffer into the span
    std::copy_n(value.data(), value.size(), buffer + offset);
}

/*
 *  DataBuffer::SetValue()
 *
 *  Description:
 *      This function will place the given value into the buffer at the
 *      given offset.
 *
 *  Parameters:
 *      value [in]
 *          The span of octets to insert into the data buffer.
 *
 *      offset [in]
 *          The offset into the buffer at which the octets will be inserted.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(const std::span<const char> value, std::size_t offset)
{
    // This library assumes a character is 8 bits
    static_assert(CHAR_BIT == 8);

    SetValue(
        { reinterpret_cast<const std::uint8_t *>(value.data()), value.size() },
        offset);
}

/*
 *  DataBuffer::SetValue()
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
 *          The offset into the buffer at which the values will be inserted.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(std::uint8_t value, std::size_t offset)
{
    // Ensure this operation will not write beyond the buffer
    if (offset >= buffer_size)
    {
        throw DataBufferException("Attempt to write beyond the buffer");
    }

    // Put the value into the buffer
    buffer[offset] = value;
}

/*
 *  DataBuffer::SetValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(std::int8_t value, std::size_t offset)
{
    // Ensure this operation will not write beyond the buffer
    if (offset >= buffer_size)
    {
        throw DataBufferException("Attempt to write beyond the buffer");
    }

    // Put the value into the buffer
    buffer[offset] = static_cast<std::uint8_t>(value);
}

/*
 *  DataBuffer::SetValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(std::uint16_t value, std::size_t offset)
{
    value = BitUtil::NetworkByteOrder(value);

    SetValue({ reinterpret_cast<const std::uint8_t *>(&value), sizeof(value) },
             offset);
}

/*
 *  DataBuffer::SetValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(std::int16_t value, std::size_t offset)
{
    SetValue(static_cast<uint16_t>(value), offset);
}

/*
 *  DataBuffer::SetValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(std::uint32_t value, std::size_t offset)
{
    value = BitUtil::NetworkByteOrder(value);

    SetValue({ reinterpret_cast<const std::uint8_t *>(&value), sizeof(value) },
             offset);
}

/*
 *  DataBuffer::SetValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(std::int32_t value, std::size_t offset)
{
    SetValue(static_cast<std::uint32_t>(value), offset);
}

/*
 *  DataBuffer::SetValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(std::uint64_t value, std::size_t offset)
{
    value = BitUtil::NetworkByteOrder(value);
    SetValue({ reinterpret_cast<const std::uint8_t *>(&value), sizeof(value) },
             offset);
}

/*
 *  DataBuffer::SetValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(std::int64_t value, std::size_t offset)
{
    SetValue(static_cast<std::uint64_t>(value), offset);
}

/*
 *  DataBuffer::SetValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(float value, std::size_t offset)
{
    // Ensure the assumption that a float is 32 bits in length
    static_assert(sizeof(value) == 4, "Float values are not the expected size");

    std::uint32_t binary32{};

    std::copy_n(reinterpret_cast<std::uint8_t *>(&value),
                sizeof(binary32),
                reinterpret_cast<std::uint8_t *>(&binary32));

    SetValue(binary32, offset);
}

/*
 *  DataBuffer::SetValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::SetValue(double value, std::size_t offset)
{
    // Ensure the assumption that a float is 64 bits in length
    static_assert(sizeof(value) == 8,
                  "Double values are not the expected size");

    std::uint64_t binary64{};

    std::copy_n(reinterpret_cast<std::uint8_t *>(&value),
                sizeof(binary64),
                reinterpret_cast<std::uint8_t *>(&binary64));

    SetValue(binary64, offset);
}

/*
 *  DataBuffer::GetValue()
 *
 *  Description:
 *      This function will read octets from the buffer at the given offset
 *      and place them in the span "value".
 *
 *  Parameters:
 *      value [out]
 *          The span into which octets will be copied out of the data
 *          buffer.
 *
 *      offset [in]
 *          The offset into the buffer from which the octets will be read.
 *
 *  Returns:
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(std::span<std::uint8_t> value,
                          std::size_t offset) const
{
    // If there is nothing to read, just return
    if (value.empty()) return;

    // Ensure this operation will not read beyond the buffer
    if ((offset + value.size()) > buffer_size)
    {
        throw DataBufferException("Attempt to read beyond the buffer");
    }

    // Copy the octets from the data buffer into the span
    std::copy_n(buffer + offset, value.size(), value.data());
}

/*
 *  DataBuffer::GetValue()
 *
 *  Description:
 *      This function will read octets from the buffer at the given offset
 *      and place them in the span referred to by "value".
 *
 *  Parameters:
 *      value [out]
 *          The span into which octets will be copied out of the data
 *          buffer.
 *
 *      offset [in]
 *          The offset into the buffer from which the octets will be read.
 *
 *  Returns:
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(std::span<char> value, std::size_t offset) const
{
    // If there is nothing to read, just return
    if (value.empty()) return;

    // Ensure this operation will not read beyond the buffer
    if ((offset + value.size()) > buffer_size)
    {
        throw DataBufferException("Attempt to read beyond the buffer");
    }

    // Copy the octets from the data buffer into the span
    std::copy_n(buffer + offset,
                value.size(),
                reinterpret_cast<std::uint8_t *>(value.data()));
}

/*
 *  DataBuffer::GetValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(std::uint8_t &value, std::size_t offset) const
{
    // Ensure this operation will not read beyond the buffer
    if (offset >= buffer_size)
    {
        throw DataBufferException("Attempt to read beyond the buffer");
    }

    // Assign the value
    value = buffer[offset];
}

/*
 *  DataBuffer::GetValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(std::int8_t &value, std::size_t offset) const
{
    // Ensure this operation will not read beyond the buffer
    if (offset >= buffer_size)
    {
        throw DataBufferException("Attempt to read beyond the buffer");
    }

    value = static_cast<std::int8_t>(buffer[offset]);
}

/*
 *  DataBuffer::GetValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(std::uint16_t &value, std::size_t offset) const
{
    GetValue({ reinterpret_cast<std::uint8_t *>(&value), sizeof(value) },
             offset);
    value = BitUtil::NetworkByteOrder(value);
}

/*
 *  DataBuffer::GetValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(std::int16_t &value, std::size_t offset) const
{
    GetValue({ reinterpret_cast<std::uint8_t *>(&value), sizeof(value) },
             offset);
    value = static_cast<std::int16_t>(
        BitUtil::NetworkByteOrder(static_cast<std::uint16_t>(value)));
}

/*
 *  DataBuffer::GetValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(std::uint32_t &value, std::size_t offset) const
{
    GetValue({ reinterpret_cast<std::uint8_t *>(&value), sizeof(value) },
             offset);
    value = BitUtil::NetworkByteOrder(value);
}

/*
 *  DataBuffer::GetValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(std::int32_t &value, std::size_t offset) const
{
    GetValue({ reinterpret_cast<std::uint8_t *>(&value), sizeof(value) },
             offset);
    value = static_cast<std::int32_t>(
        BitUtil::NetworkByteOrder(static_cast<std::uint32_t>(value)));
}

/*
 *  DataBuffer::GetValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(std::uint64_t &value, std::size_t offset) const
{
    GetValue({ reinterpret_cast<std::uint8_t *>(&value), sizeof(value) },
             offset);
    value = BitUtil::NetworkByteOrder(value);
}

/*
 *  DataBuffer::GetValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(std::int64_t &value, std::size_t offset) const
{
    GetValue({ reinterpret_cast<std::uint8_t *>(&value), sizeof(value) },
             offset);
    value = static_cast<std::int64_t>(
        BitUtil::NetworkByteOrder(static_cast<std::uint64_t>(value)));
}

/*
 *  DataBuffer::GetValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(float &value, std::size_t offset) const
{
    std::uint32_t binary32{};

    GetValue(binary32, offset);

    // Copy the octets from the data buffer into the span
    std::copy_n(reinterpret_cast<std::uint8_t *>(&binary32),
                sizeof(binary32),
                reinterpret_cast<std::uint8_t *>(&value));
}

/*
 *  DataBuffer::GetValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data outside of the buffer.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::GetValue(double &value, std::size_t offset) const
{
    std::uint64_t binary64{};

    GetValue(binary64, offset);

    // Copy the octets from the data buffer into the span
    std::copy_n(reinterpret_cast<std::uint8_t *>(&binary64),
                sizeof(binary64),
                reinterpret_cast<std::uint8_t *>(&value));
}

/*
 *  DataBuffer::AppendValue()
 *
 *  Description:
 *      This function will append the given span of octets to the end of the
 *      existing data in the buffer as determined by the data length value.
 *
 *  Parameters:
 *      value [in]
 *          The span of octets to append to the end of the existing data.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(const std::span<const std::uint8_t> value)
{
    SetValue(value, data_length);
    data_length += value.size();
}

/*
 *  DataBuffer::AppendValue()
 *
 *  Description:
 *      This function will append the given span to the end of the
 *      existing data in the buffer as determined by the data length value.
 *
 *  Parameters:
 *      value [in]
 *          The span of characters to append to the end of the existing data.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(const std::span<const char> value)
{
    SetValue(value, data_length);
    data_length += value.size();
}

/*
 *  DataBuffer::AppendValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(std::uint8_t value)
{
    SetValue(value, data_length);
    data_length += sizeof(value);
}

/*
 *  DataBuffer::AppendValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(std::int8_t value)
{
    SetValue(value, data_length);
    data_length += sizeof(value);
}

/*
 *  DataBuffer::AppendValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(std::uint16_t value)
{
    SetValue(value, data_length);
    data_length += sizeof(value);
}

/*
 *  DataBuffer::AppendValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(std::int16_t value)
{
    SetValue(value, data_length);
    data_length += sizeof(value);
}

/*
 *  DataBuffer::AppendValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(std::uint32_t value)
{
    SetValue(value, data_length);
    data_length += sizeof(value);
}

/*
 *  DataBuffer::AppendValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(std::int32_t value)
{
    SetValue(value, data_length);
    data_length += sizeof(value);
}

/*
 *  DataBuffer::AppendValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(std::uint64_t value)
{
    SetValue(value, data_length);
    data_length += sizeof(value);
}

/*
 *  DataBuffer::AppendValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(std::int64_t value)
{
    SetValue(value, data_length);
    data_length += sizeof(value);
}

/*
 *  DataBuffer::AppendValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(float value)
{
    SetValue(value, data_length);
    data_length += sizeof(value);
}

/*
 *  DataBuffer::AppendValue()
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
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::AppendValue(double value)
{
    SetValue(value, data_length);
    data_length += sizeof(value);
}

/*
 *  DataBuffer::ReadValue()
 *
 *  Description:
 *      This function will read octets from the buffer at the current read
 *      position and place them in the span "value".
 *
 *  Parameters:
 *      value [out]
 *          The span into which octets will be copied out of the data buffer.
 *
 *  Returns:
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(std::span<std::uint8_t> value)
{
    if ((read_position + value.size()) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += value.size();
}

/*
 *  DataBuffer::ReadValue()
 *
 *  Description:
 *      This function will read octets from the buffer at the current read
 *      position and place them in the span "value".
 *
 *  Parameters:
 *      value [out]
 *          The span into which octets will be copied out of the data buffer.
 *
 *  Returns:
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(std::span<char> value)
{
    if ((read_position + value.size()) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += value.size();
}

/*
 *  DataBuffer::ReadValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(std::uint8_t &value)
{
    if ((read_position + sizeof(value)) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += sizeof(value);
}

/*
 *  DataBuffer::ReadValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(std::int8_t &value)
{
    if ((read_position + sizeof(value)) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += sizeof(value);
}

/*
 *  DataBuffer::ReadValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(std::uint16_t &value)
{
    if ((read_position + sizeof(value)) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += sizeof(value);
}

/*
 *  DataBuffer::ReadValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(std::int16_t &value)
{
    if ((read_position + sizeof(value)) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += sizeof(value);
}

/*
 *  DataBuffer::ReadValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(std::uint32_t &value)
{
    if ((read_position + sizeof(value)) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += sizeof(value);
}

/*
 *  DataBuffer::ReadValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(std::int32_t &value)
{
    if ((read_position + sizeof(value)) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += sizeof(value);
}

/*
 *  DataBuffer::ReadValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(std::uint64_t &value)
{
    if ((read_position + sizeof(value)) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += sizeof(value);
}

/*
 *  DataBuffer::ReadValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(std::int64_t &value)
{
    if ((read_position + sizeof(value)) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += sizeof(value);
}

/*
 *  DataBuffer::ReadValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(float &value)
{
    if ((read_position + sizeof(value)) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += sizeof(value);
}

/*
 *  DataBuffer::ReadValue()
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
 *      Nothing, though the value parameter will be populated with the requested
 *      data.  An exception will be thrown if there is a request to retrieve
 *      data beyond the data length.
 *
 *  Comments:
 *      None.
 */
void DataBuffer::ReadValue(double &value)
{
    if ((read_position + sizeof(value)) > data_length)
    {
        throw DataBufferException("Attempt to read beyond the data length");
    }

    GetValue(value, read_position);
    read_position += sizeof(value);
}

/*
 *  DataBuffer::operator<<()
 *
 *  Description:
 *      This operator will output the contents of a DataBuffer object to a
 *      stream as a hex dump.
 *
 *  Parameters:
 *      o [in]
 *          A reference to an output stream.
 *
 *      data_buffer [in]
 *          The DataBuffer object to dump.
 *
 *  Returns:
 *      A reference to the output stream.
 *
 *  Comments:
 *      None.
 */
std::ostream &operator<<(std::ostream &o, const DataBuffer &data_buffer)
{
    std::size_t octet_counter = 0;              // Octet counter
    std::string ascii_map;                      // ASCII map of octets
    std::ostringstream oss;                     // For line formation

    // There is nothing to output if there is no data
    if (data_buffer.GetDataLength() == 0) return o;

    // We want uppercase hex digits with zero-fill integers
    oss << std::hex << std::setfill('0') << std::uppercase;

    // Iterate over the data buffer contents
    for (const auto octet : data_buffer)
    {
        // Render the octet counter at the start of new lines
        if ((octet_counter % 16) == 0)
        {
            oss << std::setw(8) << octet_counter;
            oss << ":";
        }

        // Produce the hex output for this octet
        oss << " " << std::setw(2) << static_cast<unsigned>(octet);

        // Add the ASCII form to the character map
        ascii_map += ((std::isprint(octet) != 0) ? static_cast<char>(octet) : '.');

        // Produce an output line when it is complete
        if ((++octet_counter % 16) == 0)
        {
            o << oss.str() << " :" << ascii_map << ":" << std::endl;
            oss.str("");
            ascii_map.clear();
        }
    }

    // If there is a partial last line, produce it
    if ((octet_counter % 16) != 0)
    {
        ascii_map.resize(16, ' ');
        o << oss.str() << std::string((16 - (octet_counter % 16)) * 3, ' ')
          << " :" << ascii_map << ":" << std::endl;
    }

    return o;
}

} // namespace Terra::NetUtil
