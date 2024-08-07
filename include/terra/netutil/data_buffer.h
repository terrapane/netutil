/*
 *  data_buffer.h
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file defines the DataBuffer object.  This object can be used to
 *      facilitate serializing or deserializing data in network byte order into
 *      and out of a buffer.  The DataBuffer can allocate memory to use
 *      or the user may provide an existing buffer of memory over which
 *      the data buffer will operate.  Only the constructor that specifies
 *      a buffer size will allocate memory, while all others accept an
 *      externally allocated buffer pointer or span over which to operate.
 *      The copy assignment operator will also allocate its own buffer when
 *      copying from a different DataBuffer object.
 *
 *      If DataBuffer allocates memory, it will free that memory on destruction
 *      or reassignment.  However, it will not free memory if given an existing
 *      buffer or span to utilize (i.e., it will not free memory it does not
 *      allocate).
 *
 *      In addition to easily ensuring proper byte order of data placed into
 *      the buffer, the DataBuffer has checks to ensure it is not possible
 *      to read or write outside the buffer.  Any attempt to write beyond the
 *      size of the data buffer will result in an exception thrown.
 *
 *      Along with a buffer size, there is a data length maintained that
 *      indicates how much data is in the buffer and a read position that
 *      indicates where in the buffer the user left off deserializing data.
 *      However, not all read/write operations give consideration to the data
 *      length or read position.  The four groups of read/write operations are
 *      as follows:
 *
 *          SetValue()      Sets a value in the buffer at a given location
 *                          without respect to the data length or read position.
 *
 *          GetValue()      Gets a value in the buffer at a given location
 *                          without respect to the data length or read position.
 *
 *          AppendValue()   Appends a value to the buffer, updating the
 *                          data length upon success.
 *
 *          ReadValue()     Reads a value from the data buffer with respect to
 *                          to the current read position and data length,
 *                          updating the read position on success.
 *
 *      Calling GetBufferPointer() returns a pointer into the buffer without
 *      respect to the length of the underlying data (i.e., data length and
 *      read position are ignored), though the offset is checked to ensure a
 *      returned pointer is not beyond the end of the buffer.
 *
 *      Calling GetBufferSpan() returns a span over the DataBuffer with respect
 *      to the current read position and data length.
 *
 *      Numeric values are written to the DataBuffer in Network Byte Order
 *      (big endian).  Likewise, numeric values in the DataBuffer are read
 *      in Network Byte Order and converted to host byte order.  That is
 *      primarily the reason for so many different functions to read and
 *      write values.
 *
 *      Streaming operators operator<< and operator>> exist to parallel the
 *      AppendValue() and ReadValue() functions.  Those functions allow one to
 *      utilize the DataBuffer object in a more natural way.
 *
 *      There are also [] operators for reading and writing single octets that
 *      do so without regard to the data length.  They allow the DataBuffer
 *      to be treated like any other C++ container or C array.
 *
 *      All functions will observe the actual buffer size and throw an exception
 *      if an attempt is made to read or write beyond the actual underlying
 *      buffer.
 *
 *  Portability Issues:
 *      None.
 */

#pragma once

#include <stdexcept>
#include <cstdlib>
#include <cstdint>
#include <span>
#include <ostream>
#include <limits>

namespace Terra::NetUtil
{

// Define an exception that will be thrown if an attempt is made to access
// memory outside the underlying memory buffer
class DataBufferException : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

// Define the DataBuffer object
class DataBuffer
{
    public:
        DataBuffer();
        DataBuffer(std::size_t buffer_size);
        DataBuffer(std::span<std::uint8_t> buffer);
        DataBuffer(std::uint8_t *buffer,
                   std::size_t buffer_size,
                   std::size_t data_length = 0);
        DataBuffer(const DataBuffer &other);
        DataBuffer(DataBuffer &&other) noexcept;
        virtual ~DataBuffer();

        DataBuffer &operator=(const DataBuffer &other);
        DataBuffer &operator=(DataBuffer &&other) noexcept;

        std::uint8_t *GetBufferPointer(std::size_t offset = 0) const;
        std::span<std::uint8_t> GetBufferSpan() const;
        std::size_t GetBufferSize() const;
        void SetBuffer(std::span<std::uint8_t> new_buffer);
        void SetBuffer(std::uint8_t *new_buffer,
                       std::size_t new_buffer_size,
                       std::size_t new_data_length = 0);

        std::size_t GetDataLength() const;
        void SetDataLength(std::size_t length);
        bool Empty() const;

        std::size_t GetReadPosition() const;
        void SetReadPosition(std::size_t position);
        void AdvanceReadPosition(std::size_t distance);
        std::size_t GetUnreadLength() const;

        bool operator==(const DataBuffer &other);
        bool operator!=(const DataBuffer &other);

        std::uint8_t &operator[](std::size_t index);
        const std::uint8_t &operator[](std::size_t index) const;

        std::span<std::uint8_t>::iterator begin() const noexcept;
        std::span<std::uint8_t>::iterator end() const noexcept;

        void SetValue(const std::span<const std::uint8_t> value,
                      std::size_t offset);
        void SetValue(const std::span<const char> value, std::size_t offset);
        void SetValue(std::uint8_t value, std::size_t offset);
        void SetValue(std::int8_t value, std::size_t offset);
        void SetValue(std::uint16_t value, std::size_t offset);
        void SetValue(std::int16_t value, std::size_t offset);
        void SetValue(std::uint32_t value, std::size_t offset);
        void SetValue(std::int32_t value, std::size_t offset);
        void SetValue(std::uint64_t value, std::size_t offset);
        void SetValue(std::int64_t value, std::size_t offset);
        void SetValue(float value, std::size_t offset);
        void SetValue(double value, std::size_t offset);

        void GetValue(std::span<std::uint8_t> value, std::size_t offset) const;
        void GetValue(std::span<char> value, std::size_t offset) const;
        void GetValue(std::uint8_t &value, std::size_t offset) const;
        void GetValue(std::int8_t &value, std::size_t offset) const;
        void GetValue(std::uint16_t &value, std::size_t offset) const;
        void GetValue(std::int16_t &value, std::size_t offset) const;
        void GetValue(std::uint32_t &value, std::size_t offset) const;
        void GetValue(std::int32_t &value, std::size_t offset) const;
        void GetValue(std::uint64_t &value, std::size_t offset) const;
        void GetValue(std::int64_t &value, std::size_t offset) const;
        void GetValue(float &value, std::size_t offset) const;
        void GetValue(double &value, std::size_t offset) const;

        void AppendValue(const std::span<const std::uint8_t> value);
        void AppendValue(const std::span<const char> value);
        void AppendValue(std::uint8_t value);
        void AppendValue(std::int8_t value);
        void AppendValue(std::uint16_t value);
        void AppendValue(std::int16_t value);
        void AppendValue(std::uint32_t value);
        void AppendValue(std::int32_t value);
        void AppendValue(std::uint64_t value);
        void AppendValue(std::int64_t value);
        void AppendValue(float value);
        void AppendValue(double value);

        void ReadValue(std::span<std::uint8_t> value);
        void ReadValue(std::span<char> value);
        void ReadValue(std::uint8_t &value);
        void ReadValue(std::int8_t &value);
        void ReadValue(std::uint16_t &value);
        void ReadValue(std::int16_t &value);
        void ReadValue(std::uint32_t &value);
        void ReadValue(std::int32_t &value);
        void ReadValue(std::uint64_t &value);
        void ReadValue(std::int64_t &value);
        void ReadValue(float &value);
        void ReadValue(double &value);

        // Streaming operators that call function AppendValue / ReadValue
        template<typename T>
        DataBuffer &operator<<(const T &value)
        {
            AppendValue(value);
            return *this;
        }
        template<typename T>
        DataBuffer &operator>>(T &value)
        {
            ReadValue(value);
            return *this;
        }

    protected:
        void AllocateBuffer(std::size_t buffer_size);
        void FreeBuffer();

        bool owns_buffer;                       // Is the buffer owned?
        std::uint8_t *buffer;                   // Pointer to buffer
        std::size_t buffer_size;                // Size of buffer
        std::size_t data_length;                // Length of data in buffer
        std::size_t read_position;              // Current read position
};

// Produce a hex dump of the DataBuffer contents
std::ostream &operator<<(std::ostream &o, const DataBuffer &data_buffer);

} // namespace Terra::NetUtil
