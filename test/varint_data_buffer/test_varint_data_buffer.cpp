/*
 *  test_varint_data_buffer.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements unit tests for the VarIntDataBuffer object.
 *
 *  Portability Issues:
 *      None.
 */

#include <span>
#include <cstdint>
#include <sstream>
#include <limits>
#include <terra/netutil/varint_data_buffer.h>
#include <terra/stf/stf.h>

using namespace Terra;

std::size_t SpanReceiver(std::span<const std::uint8_t> buffer)
{
    return buffer.size();
}

STF_TEST(TestDataBuffer, SetValueVarUint)
{
    NetUtil::VarUint64_t value;
    NetUtil::VarIntDataBuffer data_buffer(128);

    // Initialize the buffer
    for (std::size_t i = 0; i < 128; i++) data_buffer[i] = 0x22;

    // Populate the data buffer

    // Single octet tests
    value = 0x00;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x22, data_buffer[1]); // Should have no data

    value = 0x01;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x22, data_buffer[1]); // Should have no data

    value = 0x40;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x40, data_buffer[0]);
    STF_ASSERT_EQ(0x22, data_buffer[1]); // Should have no data

    // Two octet test
    value = 0x80;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x81, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    value = 0x100;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x82, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    value = 0x1000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xa0, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    value = 0x2000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xc0, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    // Three octet test
    value = 0x4000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x81, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x22, data_buffer[3]); // Should have no data

    value = 0x4001;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x81, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x01, data_buffer[2]);
    STF_ASSERT_EQ(0x22, data_buffer[3]); // Should have no data

    value = 0x10'0000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xc0, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x22, data_buffer[3]); // Should have no data

    // Four octet test
    value = 0x20'0000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x81, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x80, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0x22, data_buffer[4]); // Should have no data

    // Nine octet test
    value = 0x4000000000000000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xc0, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x80, data_buffer[2]);
    STF_ASSERT_EQ(0x80, data_buffer[3]);
    STF_ASSERT_EQ(0x80, data_buffer[4]);
    STF_ASSERT_EQ(0x80, data_buffer[5]);
    STF_ASSERT_EQ(0x80, data_buffer[6]);
    STF_ASSERT_EQ(0x80, data_buffer[7]);
    STF_ASSERT_EQ(0x00, data_buffer[8]);
    STF_ASSERT_EQ(0x22, data_buffer[9]); // Should have no data

    // Ten octet test
    value = 0x8000000000000000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x81, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x80, data_buffer[2]);
    STF_ASSERT_EQ(0x80, data_buffer[3]);
    STF_ASSERT_EQ(0x80, data_buffer[4]);
    STF_ASSERT_EQ(0x80, data_buffer[5]);
    STF_ASSERT_EQ(0x80, data_buffer[6]);
    STF_ASSERT_EQ(0x80, data_buffer[7]);
    STF_ASSERT_EQ(0x80, data_buffer[8]);
    STF_ASSERT_EQ(0x00, data_buffer[9]);
    STF_ASSERT_EQ(0x22, data_buffer[10]); // Should have no data

    // Largest value
    value = 0xffff'ffff'ffff'ffff;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x81, data_buffer[0]);
    STF_ASSERT_EQ(0xff, data_buffer[1]);
    STF_ASSERT_EQ(0xff, data_buffer[2]);
    STF_ASSERT_EQ(0xff, data_buffer[3]);
    STF_ASSERT_EQ(0xff, data_buffer[4]);
    STF_ASSERT_EQ(0xff, data_buffer[5]);
    STF_ASSERT_EQ(0xff, data_buffer[6]);
    STF_ASSERT_EQ(0xff, data_buffer[7]);
    STF_ASSERT_EQ(0xff, data_buffer[8]);
    STF_ASSERT_EQ(0x7f, data_buffer[9]);
    STF_ASSERT_EQ(0x22, data_buffer[10]); // Should have no data
}

STF_TEST(TestDataBuffer, SetValueVarIntPositive)
{
    NetUtil::VarInt64_t value;
    NetUtil::VarIntDataBuffer data_buffer(128);

    // Initialize the buffer
    for (std::size_t i = 0; i < 128; i++) data_buffer[i] = 0x22;

    // Single octet tests
    value = 0x00;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x22, data_buffer[1]); // Should have no data

    value = 0x01;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x22, data_buffer[1]); // Should have no data

    value = 0x20;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x20, data_buffer[0]);
    STF_ASSERT_EQ(0x22, data_buffer[1]); // Should have no data

    // Two octet test
    value = 0x40;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x80, data_buffer[0]);
    STF_ASSERT_EQ(0x40, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    value = 0x80;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x81, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    value = 0x100;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x82, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    value = 0x1000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xA0, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    // Three octet test
    value = 0x2000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x80, data_buffer[0]);
    STF_ASSERT_EQ(0xc0, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x22, data_buffer[3]); // Should have no data

    value = 0x4000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x81, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x22, data_buffer[3]); // Should have no data

    value = 0x8'0000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xa0, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x22, data_buffer[3]); // Should have no data

    // Four octet test
    value = 0x10'0000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x80, data_buffer[0]);
    STF_ASSERT_EQ(0xc0, data_buffer[1]);
    STF_ASSERT_EQ(0x80, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0x22, data_buffer[4]); // Should have no data

    value = 0x20'0000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x81, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x80, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0x22, data_buffer[4]); // Should have no data

    value = 0x40'0000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x82, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x80, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0x22, data_buffer[4]); // Should have no data

    // Nine octet test
    value = 0x2000000000000000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xa0, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x80, data_buffer[2]);
    STF_ASSERT_EQ(0x80, data_buffer[3]);
    STF_ASSERT_EQ(0x80, data_buffer[4]);
    STF_ASSERT_EQ(0x80, data_buffer[5]);
    STF_ASSERT_EQ(0x80, data_buffer[6]);
    STF_ASSERT_EQ(0x80, data_buffer[7]);
    STF_ASSERT_EQ(0x00, data_buffer[8]);
    STF_ASSERT_EQ(0x22, data_buffer[9]); // Should have no data

    // Ten octet test
    value = 0x4000000000000000;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x80, data_buffer[0]);
    STF_ASSERT_EQ(0xc0, data_buffer[1]);
    STF_ASSERT_EQ(0x80, data_buffer[2]);
    STF_ASSERT_EQ(0x80, data_buffer[3]);
    STF_ASSERT_EQ(0x80, data_buffer[4]);
    STF_ASSERT_EQ(0x80, data_buffer[5]);
    STF_ASSERT_EQ(0x80, data_buffer[6]);
    STF_ASSERT_EQ(0x80, data_buffer[7]);
    STF_ASSERT_EQ(0x80, data_buffer[8]);
    STF_ASSERT_EQ(0x00, data_buffer[9]);
    STF_ASSERT_EQ(0x22, data_buffer[10]); // Should have no data

    // Test largest signed integer
    value = std::numeric_limits<std::int64_t>::max();
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x80, data_buffer[0]);
    STF_ASSERT_EQ(0xff, data_buffer[1]);
    STF_ASSERT_EQ(0xff, data_buffer[2]);
    STF_ASSERT_EQ(0xff, data_buffer[3]);
    STF_ASSERT_EQ(0xff, data_buffer[4]);
    STF_ASSERT_EQ(0xff, data_buffer[5]);
    STF_ASSERT_EQ(0xff, data_buffer[6]);
    STF_ASSERT_EQ(0xff, data_buffer[7]);
    STF_ASSERT_EQ(0xff, data_buffer[8]);
    STF_ASSERT_EQ(0x7f, data_buffer[9]);
    STF_ASSERT_EQ(0x22, data_buffer[10]); // Should have no data
}

STF_TEST(TestDataBuffer, SetValueVarIntNegative)
{
    NetUtil::VarInt64_t value;
    NetUtil::VarIntDataBuffer data_buffer(128);

    // Initialize the buffer
    for (std::size_t i = 0; i < 128; i++) data_buffer[i] = 0x22;

    // Single octet tests
    value = -1;
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x7f, data_buffer[0]);
    STF_ASSERT_EQ(0x22, data_buffer[1]); // Should have no data

    value = -33; // 0xFFFFFFFFFFFFFFDF
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0x5f, data_buffer[0]);
    STF_ASSERT_EQ(0x22, data_buffer[1]); // Should have no data

    // Two octet tests
    value = -65; // 0xFFFFFFFFFFFFFFBF
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xff, data_buffer[0]);
    STF_ASSERT_EQ(0x3f, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    value = -129; // 0xFFFFFFFFFFFFFFBF
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xfe, data_buffer[0]);
    STF_ASSERT_EQ(0x7f, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    value = -4097; // 0xFFFFFFFFFFFFEFFF
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xdf, data_buffer[0]);
    STF_ASSERT_EQ(0x7f, data_buffer[1]);
    STF_ASSERT_EQ(0x22, data_buffer[2]); // Should have no data

    // Three octet tests
    value = -8193; // 0xFFFFFFFFFFFFDFFF
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xff, data_buffer[0]);
    STF_ASSERT_EQ(0xbf, data_buffer[1]);
    STF_ASSERT_EQ(0x7f, data_buffer[2]);
    STF_ASSERT_EQ(0x22, data_buffer[3]); // Should have no data

    value = -16385; // 0xFFFFFFFFFFFFBFFF
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xfe, data_buffer[0]);
    STF_ASSERT_EQ(0xff, data_buffer[1]);
    STF_ASSERT_EQ(0x7f, data_buffer[2]);
    STF_ASSERT_EQ(0x22, data_buffer[3]); // Should have no data

    value = -32769; // 0xFFFFFFFFFFFF7FFF
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xfd, data_buffer[0]);
    STF_ASSERT_EQ(0xff, data_buffer[1]);
    STF_ASSERT_EQ(0x7f, data_buffer[2]);
    STF_ASSERT_EQ(0x22, data_buffer[3]); // Should have no data

    // Nine octet test
    value = 0xDFFFFFFFFFFFFFFF; // Bit 61 (0..63)
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xdf, data_buffer[0]);
    STF_ASSERT_EQ(0xff, data_buffer[1]);
    STF_ASSERT_EQ(0xff, data_buffer[2]);
    STF_ASSERT_EQ(0xff, data_buffer[3]);
    STF_ASSERT_EQ(0xff, data_buffer[4]);
    STF_ASSERT_EQ(0xff, data_buffer[5]);
    STF_ASSERT_EQ(0xff, data_buffer[6]);
    STF_ASSERT_EQ(0xff, data_buffer[7]);
    STF_ASSERT_EQ(0x7f, data_buffer[8]);
    STF_ASSERT_EQ(0x22, data_buffer[9]); // Should have no data

    // Ten octet test
    value = 0xBFFFFFFFFFFFFFFF; // Bit 62 (0..63)
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xff, data_buffer[0]);
    STF_ASSERT_EQ(0xbf, data_buffer[1]);
    STF_ASSERT_EQ(0xff, data_buffer[2]);
    STF_ASSERT_EQ(0xff, data_buffer[3]);
    STF_ASSERT_EQ(0xff, data_buffer[4]);
    STF_ASSERT_EQ(0xff, data_buffer[5]);
    STF_ASSERT_EQ(0xff, data_buffer[6]);
    STF_ASSERT_EQ(0xff, data_buffer[7]);
    STF_ASSERT_EQ(0xff, data_buffer[8]);
    STF_ASSERT_EQ(0x7f, data_buffer[9]);
    STF_ASSERT_EQ(0x22, data_buffer[10]); // Should have no data

    // Test smallest signed integer
    value = std::numeric_limits<std::int64_t>::min();
    data_buffer.SetValue(value, 0);
    STF_ASSERT_EQ(0xff, data_buffer[0]);
    STF_ASSERT_EQ(0x80, data_buffer[1]);
    STF_ASSERT_EQ(0x80, data_buffer[2]);
    STF_ASSERT_EQ(0x80, data_buffer[3]);
    STF_ASSERT_EQ(0x80, data_buffer[4]);
    STF_ASSERT_EQ(0x80, data_buffer[5]);
    STF_ASSERT_EQ(0x80, data_buffer[6]);
    STF_ASSERT_EQ(0x80, data_buffer[7]);
    STF_ASSERT_EQ(0x80, data_buffer[8]);
    STF_ASSERT_EQ(0x00, data_buffer[9]);
    STF_ASSERT_EQ(0x22, data_buffer[10]); // Should have no data
}

STF_TEST(TestDataBuffer, AppendValueVarUint)
{
    NetUtil::VarUint64_t value;
    NetUtil::VarIntDataBuffer data_buffer(128);

    // Initialize the buffer
    for (std::size_t i = 0; i < 128; i++) data_buffer[i] = 0x22;

    // Assign value
    value = 0x20'0000;

    // Append a two octets (to test data length update)
    data_buffer.AppendValue(std::uint16_t(0x0001));

    // Append the value
    STF_ASSERT_EQ(4, data_buffer.AppendValue(value));

    // Verify the buffer contents

    STF_ASSERT_EQ(6, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    // The 16-bit integer
    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);

    // The VarUint value
    STF_ASSERT_EQ(0x81, data_buffer[2]);
    STF_ASSERT_EQ(0x80, data_buffer[3]);
    STF_ASSERT_EQ(0x80, data_buffer[4]);
    STF_ASSERT_EQ(0x00, data_buffer[5]);

    // Beyond what was appended should be 0x22
    STF_ASSERT_EQ(0x22, data_buffer[6]);
}

STF_TEST(TestDataBuffer, AppendValueVarInt)
{
    NetUtil::VarInt64_t value;
    NetUtil::VarIntDataBuffer data_buffer(128);

    // Initialize the buffer
    for (std::size_t i = 0; i < 128; i++) data_buffer[i] = 0x22;

    // Assign value
    value = -16385; // 0xFFFFFFFFFFFFBFFF

    // Append a two octets (to test data length update)
    // Note: Explicit scoping required since the compiler can get confused
    //       by integer types
    data_buffer.AppendValue(std::uint16_t(0x0001));

    // Append the value
    STF_ASSERT_EQ(3, data_buffer.AppendValue(value));

    // Verify the buffer contents

    STF_ASSERT_EQ(5, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    // The 16-bit integer
    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);

    // The VarInt value
    STF_ASSERT_EQ(0xfe, data_buffer[2]);
    STF_ASSERT_EQ(0xff, data_buffer[3]);
    STF_ASSERT_EQ(0x7f, data_buffer[4]);

    // Beyond what was appended should be 0x22
    STF_ASSERT_EQ(0x22, data_buffer[5]);
}

STF_TEST(TestDataBuffer, VarUint16)
{
    NetUtil::VarUint16_t value1 = 32768;
    NetUtil::VarUint16_t value2 = 65535;
    std::uint32_t final = 0xcafebabe;

    NetUtil::VarIntDataBuffer data_buffer(128);

    // Place the values into the buffer
    data_buffer.AppendValue(value1);
    data_buffer.AppendValue(value2);
    data_buffer.AppendValue(final);

    NetUtil::VarUint16_t check_value1{};
    NetUtil::VarUint16_t check_value2{};
    std::uint32_t check_final{};

    data_buffer.ReadValue(check_value1);
    data_buffer.ReadValue(check_value2);
    data_buffer.ReadValue(check_final);

    STF_ASSERT_EQ(value1, check_value1);
    STF_ASSERT_EQ(value2, check_value2);
    STF_ASSERT_EQ(final, check_final);
}

STF_TEST(TestDataBuffer, VarInt16Extremes)
{
    NetUtil::VarInt16_t value1 = -32768;
    NetUtil::VarInt16_t value2 = 32767;
    std::uint32_t final = 0xcafebabe;

    NetUtil::VarIntDataBuffer data_buffer(128);

    // Place the values into the buffer
    data_buffer.AppendValue(value1);
    data_buffer.AppendValue(value2);
    data_buffer.AppendValue(final);

    NetUtil::VarInt16_t check_value1{};
    NetUtil::VarInt16_t check_value2{};
    std::uint32_t check_final{};

    data_buffer.ReadValue(check_value1);
    data_buffer.ReadValue(check_value2);
    data_buffer.ReadValue(check_final);

    STF_ASSERT_EQ(value1, check_value1);
    STF_ASSERT_EQ(value2, check_value2);
    STF_ASSERT_EQ(final, check_final);
}

STF_TEST(TestDataBuffer, VarInt16Ones)
{
    NetUtil::VarInt16_t value1 = -1;
    NetUtil::VarInt16_t value2 = 1;
    std::uint32_t final = 0xcafebabe;

    NetUtil::VarIntDataBuffer data_buffer(128);

    // Place the values into the buffer
    data_buffer.AppendValue(value1);
    data_buffer.AppendValue(value2);
    data_buffer.AppendValue(final);

    NetUtil::VarInt16_t check_value1{};
    NetUtil::VarInt16_t check_value2{};
    std::uint32_t check_final{};

    data_buffer.ReadValue(check_value1);
    data_buffer.ReadValue(check_value2);
    data_buffer.ReadValue(check_final);

    STF_ASSERT_EQ(value1, check_value1);
    STF_ASSERT_EQ(value2, check_value2);
    STF_ASSERT_EQ(final, check_final);
}

STF_TEST(TestDataBuffer, VarIntException)
{
    // Write out a value larger than the 16-bit
    NetUtil::VarInt32_t value = -100000;
    std::uint32_t final = 0xcafebabe;

    NetUtil::VarIntDataBuffer data_buffer(128);

    // Place the values into the buffer
    data_buffer.AppendValue(value);
    data_buffer.AppendValue(final);

    // To throw an exception, attempt to read a 16-bit value
    NetUtil::VarInt16_t check_value{};

    auto test_func = [&] { data_buffer >> check_value; };

    STF_ASSERT_EXCEPTION_E(test_func, NetUtil::DataBufferException);
}

STF_TEST(TestDataBuffer, GetValueVarUint)
{
    NetUtil::VarUint64_t write_value;
    NetUtil::VarUint64_t read_value;
    NetUtil::VarIntDataBuffer data_buffer(128);
    std::vector<std::uint64_t> test_values =
    {
        0, 1, 0x40, 0x80, 0x100, 0x1000, 0x2000, 0x4000, 0x4001,
        0x10'0000, 0x20'0000, 0x40'0000, 0x200'0000, 0x400'0000,
        0x800'0000, 0x1000'0000,
        0x4000'0000, 0x8000'0000,
        0x0001'0000'0000, 0x0020'0000'0000,
        0x0400'0000'0000, 0x8000'0000'0000,
        0x4000'0000'0000'0000,
        0x8000'0000'0000'0000,
        0xFFFF'FFFF'FFFF'FFFF
    };

    for (auto value : test_values)
    {
        write_value = value;
        data_buffer.SetValue(write_value, 10);
        data_buffer.GetValue(read_value, 10);
        STF_ASSERT_EQ(write_value, read_value);
    }

    // Test a range of values (a few are redundant)
    for (std::uint64_t i = 0; i <= 65536; i++)
    {
        write_value = i;
        data_buffer.SetValue(write_value, 5);
        data_buffer.GetValue(read_value, 5);
        STF_ASSERT_EQ(write_value, read_value);
    }
}

STF_TEST(TestDataBuffer, GetValueVarInt)
{
    NetUtil::VarInt64_t write_value;
    NetUtil::VarInt64_t read_value;
    NetUtil::VarIntDataBuffer data_buffer(128);
    std::vector<std::int64_t> test_values =
    {
        // positive values
        0x00, 0x01, 0x20, 0x40, 0x80, 0x100, 0x1000, 0x2000, 0x4000,
        0x8'0000, 0x10'0000, 0x20'0000,  0x40'0000,
        0x2000000000000000, 0x4000000000000000

        // negative values
        -1, -33, -65, -129, -4097, -8193, -16385, -32769,
        -2305843009213693953, -4611686018427387905
    };

    for (auto value : test_values)
    {
        write_value = value;
        data_buffer.SetValue(write_value, 10);
        data_buffer.GetValue(read_value, 10);
        STF_ASSERT_EQ(write_value, read_value);
    }

    // Test a range of values (a few are redundant)
    for (std::int64_t i = -65536; i <= 65536; i++)
    {
        write_value = i;
        data_buffer.SetValue(write_value, 10);
        data_buffer.GetValue(read_value, 10);
        STF_ASSERT_EQ(write_value, read_value);
    }
}

STF_TEST(TestDataBuffer, ReadValueVarUint)
{
    NetUtil::VarUint64_t value;

    // Populate the data buffer
    NetUtil::VarIntDataBuffer data_buffer(128);

    // Initialize the buffer
    for (std::size_t i = 0; i < 128; i++) data_buffer[i] = 0x22;

    // Assign value
    value = 0x20'0000;

    // Append a two octets (to test data length update)
    data_buffer.AppendValue(std::uint16_t(0x1234));

    // Append the value
    STF_ASSERT_EQ(4, data_buffer.AppendValue(value));
    STF_ASSERT_EQ(6, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    // Verify the buffer contents

    // The 16-bit integer
    STF_ASSERT_EQ(0x12, data_buffer[0]);
    STF_ASSERT_EQ(0x34, data_buffer[1]);

    // The VarUint value
    STF_ASSERT_EQ(0x81, data_buffer[2]);
    STF_ASSERT_EQ(0x80, data_buffer[3]);
    STF_ASSERT_EQ(0x80, data_buffer[4]);
    STF_ASSERT_EQ(0x00, data_buffer[5]);

    // Beyond what was appended should be 0x22
    STF_ASSERT_EQ(0x22, data_buffer[6]);

    // Now read the values in the buffer
    value = 0;

    std::uint16_t dummy;
    data_buffer.ReadValue(dummy);
    STF_ASSERT_EQ(0x1234, dummy);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(0x20'0000, value);
    STF_ASSERT_EQ(6, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, ReadValueVarInt)
{
    NetUtil::VarInt64_t value;

    // Populate the data buffer
    NetUtil::VarIntDataBuffer data_buffer(128);

    // Initialize the buffer
    for (std::size_t i = 0; i < 128; i++) data_buffer[i] = 0x22;

    // Assign value
    value = -16385; // 0xFFFFFFFFFFFFBFFF

    // Append a two octets (to test data length update)
    data_buffer.AppendValue(std::uint16_t(0x1234));

    // Append the value
    STF_ASSERT_EQ(3, data_buffer.AppendValue(value));
    STF_ASSERT_EQ(5, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    // Verify the buffer contents
    STF_ASSERT_EQ(5, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    // The 16-bit integer
    STF_ASSERT_EQ(0x12, data_buffer[0]);
    STF_ASSERT_EQ(0x34, data_buffer[1]);

    // The VarInt value
    STF_ASSERT_EQ(0xfe, data_buffer[2]);
    STF_ASSERT_EQ(0xff, data_buffer[3]);
    STF_ASSERT_EQ(0x7f, data_buffer[4]);

    // Beyond what was appended should be 0x22
    STF_ASSERT_EQ(0x22, data_buffer[5]);

    // Now read the values in the buffer
    value = 0;

    std::uint16_t dummy;
    data_buffer.ReadValue(dummy);
    STF_ASSERT_EQ(0x1234, dummy);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(-16385, value);
    STF_ASSERT_EQ(5, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, VarUintSize)
{
    NetUtil::VarIntDataBuffer data_buffer;
    NetUtil::VarUint64_t value;

    value = 0;
    STF_ASSERT_EQ(std::size_t(1), data_buffer.VarUintSize(value));

    value = 1;
    STF_ASSERT_EQ(std::size_t(1), data_buffer.VarUintSize(value));

    value = 0x40;
    STF_ASSERT_EQ(std::size_t(1), data_buffer.VarUintSize(value));

    value = 0x80;
    STF_ASSERT_EQ(std::size_t(2), data_buffer.VarUintSize(value));

    value = 0x2000;
    STF_ASSERT_EQ(std::size_t(2), data_buffer.VarUintSize(value));

    value = 0x4000;
    STF_ASSERT_EQ(std::size_t(3), data_buffer.VarUintSize(value));

    value = 0x0010'0000;
    STF_ASSERT_EQ(std::size_t(3), data_buffer.VarUintSize(value));

    value = 0x0020'0000;
    STF_ASSERT_EQ(std::size_t(4), data_buffer.VarUintSize(value));

    value = 0x0800'0000;
    STF_ASSERT_EQ(std::size_t(4), data_buffer.VarUintSize(value));

    value = 0x1000'0000;
    STF_ASSERT_EQ(std::size_t(5), data_buffer.VarUintSize(value));

    value = 0x0004'0000'0000;
    STF_ASSERT_EQ(std::size_t(5), data_buffer.VarUintSize(value));

    value = 0x0008'0000'0000;
    STF_ASSERT_EQ(std::size_t(6), data_buffer.VarUintSize(value));

    value = 0x0200'0000'0000;
    STF_ASSERT_EQ(std::size_t(6), data_buffer.VarUintSize(value));

    value = 0x0400'0000'0000;
    STF_ASSERT_EQ(std::size_t(7), data_buffer.VarUintSize(value));

    value = 0x0001'0000'0000'0000;
    STF_ASSERT_EQ(std::size_t(7), data_buffer.VarUintSize(value));

    value = 0x0002'0000'0000'0000;
    STF_ASSERT_EQ(std::size_t(8), data_buffer.VarUintSize(value));

    value = 0x0080'0000'0000'0000;
    STF_ASSERT_EQ(std::size_t(8), data_buffer.VarUintSize(value));

    value = 0x0100'0000'0000'0000;
    STF_ASSERT_EQ(std::size_t(9), data_buffer.VarUintSize(value));

    value = 0x4000000000000000;
    STF_ASSERT_EQ(std::size_t(9), data_buffer.VarUintSize(value));

    value = 0x8000000000000000;
    STF_ASSERT_EQ(std::size_t(10), data_buffer.VarUintSize(value));

    value = 0xFFFFFFFFFFFFFFFF;
    STF_ASSERT_EQ(std::size_t(10), data_buffer.VarUintSize(value));
}

STF_TEST(TestDataBuffer, VarIntSize)
{
    NetUtil::VarIntDataBuffer data_buffer;
    NetUtil::VarInt64_t value;
    std::size_t position;
    std::size_t octets;

    // Positive values

    value = 0;
    STF_ASSERT_EQ(std::size_t(1), data_buffer.VarIntSize(value));

    value = 1;
    STF_ASSERT_EQ(std::size_t(1), data_buffer.VarIntSize(value));

    position = 0;
    octets = 1;
    value = 1;
    while (position < 56)
    {
        // Advance the MSb 5 positions
        value <<= 5;
        position += 5;
        STF_ASSERT_EQ(std::size_t(octets), data_buffer.VarIntSize(value));

        // Advance the MSb by 1 bit position, so we now need another
        // octet to hold the sign bit
        value <<= 1;
        position++;
        octets++;
        STF_ASSERT_EQ(std::size_t(octets), data_buffer.VarIntSize(value));

        // Advance the MSb by 1 bit position, moving into the next 7-bit group
        value <<= 1;
        position++;
        STF_ASSERT_TRUE(value > 0);
        STF_ASSERT_EQ(std::size_t(octets), data_buffer.VarIntSize(value));
    }

    // Advance the MSb 5 positions
    value <<= 5;
    position += 5;
    STF_ASSERT_EQ(std::size_t(octets), data_buffer.VarIntSize(value));

    // Advance the MSb by 1 bit position, so we now need another
    // octet to hold the sign bit
    value <<= 1;
    position++;
    octets++;
    STF_ASSERT_EQ(std::size_t(octets), data_buffer.VarIntSize(value));

    // We should have account for all 62 bits; bit 63 is the sign bit
    // and is always encoded next to the MSb of the most significant octet
    STF_ASSERT_EQ(62, position);

    // Ensure the largest positive number is encoded in 10 octets
    value = 0x7FFFFFFFFFFFFFFF;
    STF_ASSERT_EQ(10, data_buffer.VarIntSize(value));

    // Negative values

    value = -1;
    STF_ASSERT_EQ(std::size_t(1), data_buffer.VarIntSize(value));

    value = -2;
    STF_ASSERT_EQ(std::size_t(1), data_buffer.VarIntSize(value));

    position = 0;
    octets = 1;
    value = -2;
    while (position < 56)
    {
        // Advance the MSb 5 positions
        value <<= 5;
        value |= 0b11111; // Fill the void with 1s (not that it matters)
        position += 5;
        STF_ASSERT_EQ(std::size_t(octets), data_buffer.VarIntSize(value));

        // Advance the MSb by 1 bit position, so we now need another
        // octet to hold the sign bit
        value <<= 1;
        value |= 1; // Fill the void with 1s (not that it matters)
        position++;
        octets++;
        STF_ASSERT_EQ(std::size_t(octets), data_buffer.VarIntSize(value));

        // Advance the MSb by 1 bit position, moving into the next 7-bit group
        value <<= 1;
        value |= 1; // Fill the void with 1s (not that it matters)
        position++;
        STF_ASSERT_TRUE(value < 0);
        STF_ASSERT_EQ(std::size_t(octets), data_buffer.VarIntSize(value));
    }

    // Advance the MSb 5 positions
    value <<= 5;
    value |= 0b11111; // Fill the void with 1s (not that it matters)
    position += 5;
    STF_ASSERT_EQ(std::size_t(octets), data_buffer.VarIntSize(value));

    // Advance the MSb by 1 bit position, so we now need another
    // octet to hold the sign bit
    value <<= 1;
    value |= 1; // Fill the void with 1s (not that it matters)
    position++;
    octets++;
    STF_ASSERT_EQ(std::size_t(octets), data_buffer.VarIntSize(value));

    // We should have account for all 62 bits; bit 63 is the sign bit
    // and is always encoded next to the MSb of the most significant octet
    STF_ASSERT_EQ(62, position);

    // Ensure the smallest negative number is encoded in 10 octets
    value = 0x8000000000000000;
    STF_ASSERT_EQ(10, data_buffer.VarIntSize(value));
}

STF_TEST(TestDataBuffer, StreamingOperators)
{
    NetUtil::VarIntDataBuffer data_buffer(512);

    struct TestStruct
    {
        std::uint8_t i8;
        std::uint16_t i16;
        NetUtil::VarInt64_t vui16;
        std::uint32_t i32;
        std::uint64_t i64;
        float f32;
        double f64;
        std::vector<std::uint8_t> v = { 0, 0, 0, 0 };
        NetUtil::VarInt64_t vi64;
    };

    TestStruct original =
    {
        .i8 = 0x12,
        .i16 = 0x0003,
        .vui16 = 0xffee,
        .i32 = 0xdeadbeef,
        .i64 = 0x1112131415161718,
        .f32 = 3.25f,
        .f64 = -12.5,
        .v = { 0x10, 0x20, 0x30, 0x40 },
        .vi64 = 0x1000
    };

    data_buffer << original.i8 << original.i16 << original.vui16 << original.i32
                << original.i64 << original.f32 << original.f64  << original.v
                << original.vi64;

    STF_ASSERT_EQ(0x12, data_buffer[0]); // i8

    STF_ASSERT_EQ(0x00, data_buffer[1]); // i16
    STF_ASSERT_EQ(0x03, data_buffer[2]);

    STF_ASSERT_EQ(0x83, data_buffer[3]); // vui16
    STF_ASSERT_EQ(0xff, data_buffer[4]);
    STF_ASSERT_EQ(0x6e, data_buffer[5]);

    STF_ASSERT_EQ(0xde, data_buffer[6]); // i32
    STF_ASSERT_EQ(0xad, data_buffer[7]);
    STF_ASSERT_EQ(0xbe, data_buffer[8]);
    STF_ASSERT_EQ(0xef, data_buffer[9]);

    STF_ASSERT_EQ(0x11, data_buffer[10]); // i64
    STF_ASSERT_EQ(0x12, data_buffer[11]);
    STF_ASSERT_EQ(0x13, data_buffer[12]);
    STF_ASSERT_EQ(0x14, data_buffer[13]);
    STF_ASSERT_EQ(0x15, data_buffer[14]);
    STF_ASSERT_EQ(0x16, data_buffer[15]);
    STF_ASSERT_EQ(0x17, data_buffer[16]);
    STF_ASSERT_EQ(0x18, data_buffer[17]);

    STF_ASSERT_EQ(0x40, data_buffer[18]); // f32
    STF_ASSERT_EQ(0x50, data_buffer[19]);
    STF_ASSERT_EQ(0x00, data_buffer[20]);
    STF_ASSERT_EQ(0x00, data_buffer[21]);

    STF_ASSERT_EQ(0xc0, data_buffer[22]); // f64
    STF_ASSERT_EQ(0x29, data_buffer[23]);
    STF_ASSERT_EQ(0x00, data_buffer[24]);
    STF_ASSERT_EQ(0x00, data_buffer[25]);
    STF_ASSERT_EQ(0x00, data_buffer[26]);
    STF_ASSERT_EQ(0x00, data_buffer[27]);
    STF_ASSERT_EQ(0x00, data_buffer[28]);
    STF_ASSERT_EQ(0x00, data_buffer[29]);

    STF_ASSERT_EQ(0x10, data_buffer[30]); // v
    STF_ASSERT_EQ(0x20, data_buffer[31]);
    STF_ASSERT_EQ(0x30, data_buffer[32]);
    STF_ASSERT_EQ(0x40, data_buffer[33]);

    STF_ASSERT_EQ(0xa0, data_buffer[34]); // vi64
    STF_ASSERT_EQ(0x00, data_buffer[35]);

    // Now read from the buffer
    TestStruct output{};

    data_buffer >> output.i8 >> output.i16 >> output.vui16 >> output.i32
                >> output.i64 >> output.f32 >> output.f64 >> output.v
                >> output.vi64;

    // Ensure all read values are as expected
    STF_ASSERT_EQ(original.i8, output.i8);
    STF_ASSERT_EQ(original.i16, output.i16);
    STF_ASSERT_EQ(original.vui16, output.vui16);
    STF_ASSERT_EQ(original.i32, output.i32);
    STF_ASSERT_EQ(original.i64, output.i64);
    STF_ASSERT_EQ(original.f32, output.f32);
    STF_ASSERT_EQ(original.f64, output.f64);
    STF_ASSERT_EQ(original.v, output.v);
    STF_ASSERT_EQ(original.vi64, output.vi64);
}
