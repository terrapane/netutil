/*
 *  test_data_buffer.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements unit tests for the DataBuffer object.
 *
 *  Portability Issues:
 *      None.
 */

#include <span>
#include <cstdint>
#include <sstream>
#include <limits>
#include <terra/netutil/data_buffer.h>
#include <terra/stf/stf.h>

using namespace Terra;

std::size_t SpanReceiver(std::span<const std::uint8_t> buffer)
{
    return buffer.size();
}

STF_TEST(TestDataBuffer, Constructor1)
{
    NetUtil::DataBuffer data_buffer;

    STF_ASSERT_EQ(0, data_buffer.GetDataLength());
    STF_ASSERT_TRUE(data_buffer.Empty());
    STF_ASSERT_EQ(0, data_buffer.GetBufferSize());
    STF_ASSERT_EQ(nullptr, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, Constructor2)
{
    NetUtil::DataBuffer data_buffer(1500);

    STF_ASSERT_EQ(0, data_buffer.GetDataLength());
    STF_ASSERT_TRUE(data_buffer.Empty());
    STF_ASSERT_EQ(1500, data_buffer.GetBufferSize());
    STF_ASSERT_NE(nullptr, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, Constructor3)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64, 16);

    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());
    STF_ASSERT_NE(nullptr, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, Constructor4)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64, 16);
    NetUtil::DataBuffer data_buffer2(data_buffer); // Copy

    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());
    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());

    STF_ASSERT_EQ(16, data_buffer2.GetDataLength());
    STF_ASSERT_FALSE(data_buffer2.Empty());
    STF_ASSERT_EQ(64, data_buffer2.GetBufferSize());
    STF_ASSERT_NE(nullptr, data_buffer2.GetBufferPointer());
    STF_ASSERT_NE(buffer, data_buffer2.GetBufferPointer());
    STF_ASSERT_EQ(0, data_buffer2.GetReadPosition());
}

STF_TEST(TestDataBuffer, Constructor5)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64, 16);
    NetUtil::DataBuffer data_buffer2(std::move(data_buffer)); // Move

    // Original buffer should be empty
    STF_ASSERT_EQ(0, data_buffer.GetDataLength());
    STF_ASSERT_TRUE(data_buffer.Empty());
    STF_ASSERT_EQ(0, data_buffer.GetBufferSize());
    STF_ASSERT_EQ(nullptr, data_buffer.GetBufferPointer());

    // Receiving buffer should have original values
    STF_ASSERT_EQ(16, data_buffer2.GetDataLength());
    STF_ASSERT_FALSE(data_buffer2.Empty());
    STF_ASSERT_EQ(64, data_buffer2.GetBufferSize());
    STF_ASSERT_EQ(buffer, data_buffer2.GetBufferPointer());
}

STF_TEST(TestDataBuffer, Constructor6)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer);

    // Verify that the buffer and data length are 64 octets
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());
    STF_ASSERT_EQ(64, data_buffer.GetDataLength());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSpan().size());

    // Read two octets out of the buffer
    std::uint16_t some_value;
    data_buffer >> some_value;

    // Span size should now be 62
    STF_ASSERT_EQ(62, data_buffer.GetBufferSpan().size());
}

STF_TEST(TestDataBuffer, CopyAssignmentOperator)
{
    NetUtil::DataBuffer db1(64);
    NetUtil::DataBuffer db2(128);
    std::uint32_t value = 0xcafebabe;

    // Write "value" into db1
    db1 << value;
    STF_ASSERT_EQ(4, db1.GetDataLength());
    STF_ASSERT_EQ(0, db2.GetDataLength());
    STF_ASSERT_EQ(64, db1.GetBufferSize());
    STF_ASSERT_EQ(128, db2.GetBufferSize());

    // Copy db1 to db2
    db2 = db1;
    STF_ASSERT_EQ(4, db1.GetDataLength());
    STF_ASSERT_EQ(4, db2.GetDataLength());
    STF_ASSERT_EQ(64, db1.GetBufferSize());
    STF_ASSERT_EQ(64, db2.GetBufferSize());

    // Read the value from db2 and verify it is correct
    std::uint32_t value_read;
    db2 >> value_read;
    STF_ASSERT_EQ(value, value_read);
}

STF_TEST(TestDataBuffer, MoveAssignmentOperator)
{
    NetUtil::DataBuffer db1(64);
    NetUtil::DataBuffer db2(128);
    std::uint32_t value = 0xcafebabe;

    // Write "value" into db1
    db1 << value;
    STF_ASSERT_EQ(4, db1.GetDataLength());
    STF_ASSERT_EQ(0, db2.GetDataLength());
    STF_ASSERT_EQ(64, db1.GetBufferSize());
    STF_ASSERT_EQ(128, db2.GetBufferSize());

    // Move db1 into db2
    db2 = std::move(db1);
    STF_ASSERT_EQ(0, db1.GetDataLength());
    STF_ASSERT_EQ(4, db2.GetDataLength());
    STF_ASSERT_EQ(0, db1.GetBufferSize());
    STF_ASSERT_EQ(64, db2.GetBufferSize());

    // Read the value from db2 and verify it is correct
    std::uint32_t value_read;
    db2 >> value_read;
    STF_ASSERT_EQ(value, value_read);
}

STF_TEST(TestDataBuffer, SetBuffer1)
{
    std::uint8_t buffer[64];
    std::uint8_t buffer2[32];
    NetUtil::DataBuffer data_buffer(buffer, 64, 16);

    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());

    data_buffer.SetBuffer(std::span{buffer2, sizeof(buffer2)});

    STF_ASSERT_EQ(buffer2, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(32, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(32, data_buffer.GetBufferSize());
}

STF_TEST(TestDataBuffer, SetBuffer2)
{
    std::uint8_t buffer[64];
    std::uint8_t buffer2[32];
    NetUtil::DataBuffer data_buffer(buffer, 64, 16);

    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());

    data_buffer.SetBuffer(buffer2, 32, 8);

    STF_ASSERT_EQ(buffer2, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(8, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(32, data_buffer.GetBufferSize());
}

STF_TEST(TestDataBuffer, SetDataLength1)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64);

    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(0, data_buffer.GetDataLength());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());

    data_buffer.SetDataLength(20);

    STF_ASSERT_EQ(20, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
}

STF_TEST(TestDataBuffer, SetDataLength2)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64);

    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(0, data_buffer.GetDataLength());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());

    // THis should be OK
    data_buffer.SetDataLength(64);
    STF_ASSERT_EQ(64, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());

    bool exception_caught = false;

    // Setting the length larger than the buffer is a problem
    try
    {
        data_buffer.SetDataLength(65);
    }
    catch (const NetUtil::DataBufferException &)
    {
        exception_caught = true;
    }

    STF_ASSERT_TRUE(exception_caught);
}

STF_TEST(TestDataBuffer, SetDataLength3)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64, 32);

    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());
    STF_ASSERT_EQ(32, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    data_buffer.SetReadPosition(20);
    STF_ASSERT_EQ(20, data_buffer.GetReadPosition());

    // Changing the data length should reset the read position
    data_buffer.SetDataLength(48);
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, SetReadPosition1)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64, 16);

    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    data_buffer.SetReadPosition(4);

    STF_ASSERT_EQ(4, data_buffer.GetReadPosition());

}

STF_TEST(TestDataBuffer, SetReadPosition2)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64, 16);

    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    // Setting the read position to the data length is valid
    data_buffer.SetReadPosition(16);
}

STF_TEST(TestDataBuffer, SetReadPosition3)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64, 16);

    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    bool exception_caught = false;

    // Setting the read position beyond data length is an error
    try
    {
        data_buffer.SetReadPosition(17);
    }
    catch (const NetUtil::DataBufferException &)
    {
        exception_caught = true;
    }

    STF_ASSERT_TRUE(exception_caught);
}

STF_TEST(TestDataBuffer, AdvanceReadPosition)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64, 16);

    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    data_buffer.SetReadPosition(10);
    STF_ASSERT_EQ(10, data_buffer.GetReadPosition());

    // Should be fine
    data_buffer.AdvanceReadPosition(4);
    STF_ASSERT_EQ(14, data_buffer.GetReadPosition());

    // Should be fine
    data_buffer.AdvanceReadPosition(1);
    STF_ASSERT_EQ(15, data_buffer.GetReadPosition());

    // Should be fine
    data_buffer.AdvanceReadPosition(1);
    STF_ASSERT_EQ(16, data_buffer.GetReadPosition());

    bool exception_caught = false;

    // Once more and this should cause an error
    try
    {
        data_buffer.AdvanceReadPosition(1);
    }
    catch (const NetUtil::DataBufferException &)
    {
        exception_caught = true;
    }

    STF_ASSERT_TRUE(exception_caught);
}

STF_TEST(TestDataBuffer, GetUnreadLength)
{
    std::uint8_t buffer[64];
    NetUtil::DataBuffer data_buffer(buffer, 64, 16);

    STF_ASSERT_EQ(buffer, data_buffer.GetBufferPointer());
    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());
    STF_ASSERT_EQ(16, data_buffer.GetUnreadLength());

    // Read an integer
    unsigned i;
    data_buffer >> i;

    // Verify the unread length
    STF_ASSERT_EQ(16 - sizeof(unsigned), data_buffer.GetUnreadLength());

    // Read a single octet
    std::uint8_t j;
    data_buffer >> j;

    // Verify the unread length
    STF_ASSERT_EQ(16 - sizeof(unsigned) - 1, data_buffer.GetUnreadLength());
}

STF_TEST(TestDataBuffer, DataBufferAssignment1)
{
    std::uint8_t buffer[64];
    buffer[0] = 0x00;
    buffer[1] = 0x01;
    buffer[2] = 0x02;
    buffer[3] = 0x03;
    NetUtil::DataBuffer data_buffer(buffer, 64, 4);

    STF_ASSERT_EQ(4, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());

    NetUtil::DataBuffer data_buffer2(128);

    STF_ASSERT_EQ(0, data_buffer2.GetDataLength());
    STF_ASSERT_TRUE(data_buffer2.Empty());
    STF_ASSERT_EQ(128, data_buffer2.GetBufferSize());

    // Assign data_buffer to data_buffer2
    data_buffer2 = data_buffer;
    STF_ASSERT_EQ(4, data_buffer2.GetDataLength());
    STF_ASSERT_FALSE(data_buffer2.Empty());
    STF_ASSERT_EQ(64, data_buffer2.GetBufferSize());
    STF_ASSERT_EQ(data_buffer, data_buffer2);
}

STF_TEST(TestDataBuffer, DataBufferAssignment2)
{
    std::uint8_t buffer[64];
    buffer[0] = 0x00;
    buffer[1] = 0x01;
    buffer[2] = 0x02;
    buffer[3] = 0x03;
    NetUtil::DataBuffer data_buffer(buffer, 64, 4);

    STF_ASSERT_EQ(4, data_buffer.GetDataLength());
    STF_ASSERT_FALSE(data_buffer.Empty());
    STF_ASSERT_EQ(64, data_buffer.GetBufferSize());

    NetUtil::DataBuffer data_buffer2;   // No buffer at all

    STF_ASSERT_EQ(0, data_buffer2.GetDataLength());
    STF_ASSERT_TRUE(data_buffer2.Empty());
    STF_ASSERT_EQ(0, data_buffer2.GetBufferSize());
    STF_ASSERT_NE(data_buffer, data_buffer2);

    // Assign data_buffer to data_buffer2
    data_buffer2 = data_buffer;
    STF_ASSERT_EQ(4, data_buffer2.GetDataLength());
    STF_ASSERT_FALSE(data_buffer2.Empty());
    STF_ASSERT_EQ(64, data_buffer2.GetBufferSize());
    STF_ASSERT_EQ(data_buffer, data_buffer2);
}

STF_TEST(TestDataBuffer, CompareEqual)
{
    std::uint8_t buffer[64];
    buffer[0] = 0x00;
    buffer[1] = 0x01;
    buffer[2] = 0x02;
    buffer[3] = 0x03;
    NetUtil::DataBuffer data_buffer(buffer, 64, 4);

    std::uint8_t buffer2[32];
    buffer2[0] = 0x00;
    buffer2[1] = 0x01;
    buffer2[2] = 0x02;
    buffer2[3] = 0x03;
    NetUtil::DataBuffer data_buffer2(buffer2, 32, 4);

    STF_ASSERT_EQ(data_buffer, data_buffer2);
}

STF_TEST(TestDataBuffer, CompareNotEqual)
{
    std::uint8_t buffer[64];
    buffer[0] = 0x00;
    buffer[1] = 0x01;
    buffer[2] = 0x02;
    buffer[3] = 0x03;
    NetUtil::DataBuffer data_buffer(buffer, 64, 4);

    std::uint8_t buffer2[32];
    buffer2[0] = 0x00;
    buffer2[1] = 0x01;
    buffer2[2] = 0x02;
    buffer2[3] = 0xFF;
    NetUtil::DataBuffer data_buffer2(buffer2, 32, 4);

    STF_ASSERT_NE(data_buffer, data_buffer2);
}

STF_TEST(TestDataBuffer, OperatorIndex1)
{
    std::uint8_t buffer[64];
    buffer[0] = 0x00;
    buffer[1] = 0x01;
    buffer[2] = 0x02;
    buffer[3] = 0x03;
    NetUtil::DataBuffer data_buffer(buffer, 64, 4);

    std::uint8_t buffer2[32];
    NetUtil::DataBuffer data_buffer2(buffer2, 32);
    data_buffer2[0] = 0x00;
    data_buffer2[1] = 0x01;
    data_buffer2[2] = 0x02;
    data_buffer2[3] = 0x03;
    data_buffer2.SetDataLength(4);

    STF_ASSERT_EQ(data_buffer, data_buffer2);
}

STF_TEST(TestDataBuffer, OperatorIndex2)
{
    std::uint8_t buffer[64];
    buffer[0] = 0x00;
    buffer[1] = 0x01;
    buffer[2] = 0x02;
    buffer[3] = 0x03;
    NetUtil::DataBuffer data_buffer(buffer, 64, 4);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);
}

STF_TEST(TestDataBuffer, SetValueArray)
{
    std::uint8_t buffer[4] = { 0x00, 0x01, 0x02, 0x03 };

    NetUtil::DataBuffer data_buffer(4);

    data_buffer.SetValue(buffer, 0);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);
}

STF_TEST(TestDataBuffer, SetValueVector)
{
    std::vector<uint8_t> data = { 0x00, 0x01, 0x02, 0x03 };

    NetUtil::DataBuffer data_buffer(4);

    data_buffer.SetValue(data, 0);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);
}

STF_TEST(TestDataBuffer, SetValueString)
{
    std::string data = { 0x00, 0x01, 0x02, 0x03 };

    NetUtil::DataBuffer data_buffer(64);

    data_buffer.SetValue(data, 4);

    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x01, data_buffer[5]);
    STF_ASSERT_EQ(0x02, data_buffer[6]);
    STF_ASSERT_EQ(0x03, data_buffer[7]);
}

STF_TEST(TestDataBuffer, SetValueUint8)
{
    std::uint8_t value;
    NetUtil::DataBuffer data_buffer(64);

    value = 0, data_buffer.SetValue(value, 0);
    value = 1, data_buffer.SetValue(value, 1);
    value = 2, data_buffer.SetValue(value, 2);
    value = 3, data_buffer.SetValue(value, 3);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);
}

STF_TEST(TestDataBuffer, SetValueInt8)
{
    std::int8_t value;
    NetUtil::DataBuffer data_buffer(64);

    value =  0, data_buffer.SetValue(value, 0);
    value =  1, data_buffer.SetValue(value, 1);
    value =  2, data_buffer.SetValue(value, 2);
    value =  3, data_buffer.SetValue(value, 3);
    value = -1, data_buffer.SetValue(value, 4);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);
    STF_ASSERT_EQ(0xFF, data_buffer[4]);
}

STF_TEST(TestDataBuffer, SetValueUint16)
{
    std::uint16_t value;

    NetUtil::DataBuffer data_buffer(64);
    value = 0, data_buffer.SetValue(value, 0);
    value = 1, data_buffer.SetValue(value, 2);
    value = 2, data_buffer.SetValue(value, 4);
    value = 3, data_buffer.SetValue(value, 6);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x01, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x02, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x03, data_buffer[7]);
}

STF_TEST(TestDataBuffer, SetValueInt16)
{
    std::int16_t value;

    NetUtil::DataBuffer data_buffer(64);
    value =  0, data_buffer.SetValue(value, 0);
    value =  1, data_buffer.SetValue(value, 2);
    value =  2, data_buffer.SetValue(value, 4);
    value =  3, data_buffer.SetValue(value, 6);
    value = -2, data_buffer.SetValue(value, 8);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x01, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x02, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x03, data_buffer[7]);
    STF_ASSERT_EQ(0xFF, data_buffer[8]);
    STF_ASSERT_EQ(0xFE, data_buffer[9]);
}

STF_TEST(TestDataBuffer, SetValueUint32)
{
    std::uint32_t value;

    NetUtil::DataBuffer data_buffer(64);
    value = 0x01020304, data_buffer.SetValue(value, 0);
    value = 0x05060708, data_buffer.SetValue(value, 4);

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);
}

STF_TEST(TestDataBuffer, SetValueInt32)
{
    std::int32_t value;

    NetUtil::DataBuffer data_buffer(64);
    value = 0x01020304, data_buffer.SetValue(value, 0);
    value = -1858840968, data_buffer.SetValue(value, 4);

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x91, data_buffer[4]);
    STF_ASSERT_EQ(0x34, data_buffer[5]);
    STF_ASSERT_EQ(0x56, data_buffer[6]);
    STF_ASSERT_EQ(0x78, data_buffer[7]);
}

STF_TEST(TestDataBuffer, SetValueUint64)
{
    std::uint64_t value;

    NetUtil::DataBuffer data_buffer(64);
    value = 0x0102030405060708, data_buffer.SetValue(value, 0);
    value = 0x1112131415161718, data_buffer.SetValue(value, 8);

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);

    STF_ASSERT_EQ(0x11, data_buffer[8]);
    STF_ASSERT_EQ(0x12, data_buffer[9]);
    STF_ASSERT_EQ(0x13, data_buffer[10]);
    STF_ASSERT_EQ(0x14, data_buffer[11]);
    STF_ASSERT_EQ(0x15, data_buffer[12]);
    STF_ASSERT_EQ(0x16, data_buffer[13]);
    STF_ASSERT_EQ(0x17, data_buffer[14]);
    STF_ASSERT_EQ(0x18, data_buffer[15]);
}

STF_TEST(TestDataBuffer, SetValueInt64)
{
    std::int64_t value;

    NetUtil::DataBuffer data_buffer(64);
    value = 0x0102030405060708, data_buffer.SetValue(value, 0);
    value = -7993305411655166184, data_buffer.SetValue(value, 8);

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);

    STF_ASSERT_EQ(0x91, data_buffer[8]);
    STF_ASSERT_EQ(0x12, data_buffer[9]);
    STF_ASSERT_EQ(0x13, data_buffer[10]);
    STF_ASSERT_EQ(0x14, data_buffer[11]);
    STF_ASSERT_EQ(0x15, data_buffer[12]);
    STF_ASSERT_EQ(0x16, data_buffer[13]);
    STF_ASSERT_EQ(0x17, data_buffer[14]);
    STF_ASSERT_EQ(0x18, data_buffer[15]);
}

STF_TEST(TestDataBuffer, SetValueFloat)
{
    float value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 3.25f, data_buffer.SetValue(value, 0);
    value = -12.5f, data_buffer.SetValue(value, 4);

    STF_ASSERT_EQ(0x40, data_buffer[0]);
    STF_ASSERT_EQ(0x50, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0xc1, data_buffer[4]);
    STF_ASSERT_EQ(0x48, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x00, data_buffer[7]);
}

STF_TEST(TestDataBuffer, SetValueDouble)
{
    double value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 3.25, data_buffer.SetValue(value, 0);
    value = -12.5, data_buffer.SetValue(value, 8);

    STF_ASSERT_EQ(0x40, data_buffer[0]);
    STF_ASSERT_EQ(0x0a, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x00, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x00, data_buffer[7]);
    STF_ASSERT_EQ(0xc0, data_buffer[8]);
    STF_ASSERT_EQ(0x29, data_buffer[9]);
    STF_ASSERT_EQ(0x00, data_buffer[10]);
    STF_ASSERT_EQ(0x00, data_buffer[11]);
    STF_ASSERT_EQ(0x00, data_buffer[12]);
    STF_ASSERT_EQ(0x00, data_buffer[13]);
    STF_ASSERT_EQ(0x00, data_buffer[14]);
    STF_ASSERT_EQ(0x00, data_buffer[15]);
}

STF_TEST(TestDataBuffer, GetValueArray)
{
    std::uint8_t buffer[4] = { 0x00, 0x01, 0x02, 0x03 };
    std::uint8_t buffer_read[4] = { 0xff, 0xff, 0xff, 0xff };

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(4);
    data_buffer.SetValue(buffer, 0);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);

    // Read from the DataBuffer
    data_buffer.GetValue(buffer_read, 0);

    STF_ASSERT_EQ(buffer[0], buffer_read[0]);
    STF_ASSERT_EQ(buffer[1], buffer_read[1]);
    STF_ASSERT_EQ(buffer[2], buffer_read[2]);
    STF_ASSERT_EQ(buffer[3], buffer_read[3]);
}

STF_TEST(TestDataBuffer, GetValueVector)
{
    std::vector<std::uint8_t> vector_write = { 0x00, 0x01, 0x02, 0x03 };
    std::vector<std::uint8_t> vector_read(4);

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(4);
    data_buffer.SetValue(vector_write, 0);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);

    // Read from the DataBuffer
    data_buffer.GetValue(vector_read, 0);

    STF_ASSERT_EQ(vector_write[0], vector_read[0]);
    STF_ASSERT_EQ(vector_write[1], vector_read[1]);
    STF_ASSERT_EQ(vector_write[2], vector_read[2]);
    STF_ASSERT_EQ(vector_write[3], vector_read[3]);
}

STF_TEST(TestDataBuffer, GetValueString)
{
    std::string string_write = { 0x00, 0x01, 0x02, 0x03 };
    std::string string_read(4, 0x7f);

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(4);
    data_buffer.SetValue(string_write, 0);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);

    // Read from the DataBuffer
    data_buffer.GetValue(string_read, 0);

    STF_ASSERT_EQ(string_write[0], string_read[0]);
    STF_ASSERT_EQ(string_write[1], string_read[1]);
    STF_ASSERT_EQ(string_write[2], string_read[2]);
    STF_ASSERT_EQ(string_write[3], string_read[3]);
}

STF_TEST(TestDataBuffer, GetValueUint8)
{
    std::uint8_t buffer[4] = { 0x00, 0x01, 0x02, 0x03 };
    std::uint8_t value;

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(4);
    data_buffer.SetValue(buffer, 0);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);

    // Read from the DataBuffer
    data_buffer.GetValue(value, 0);
    STF_ASSERT_EQ(buffer[0], value);
    data_buffer.GetValue(value, 1);
    STF_ASSERT_EQ(buffer[1], value);
    data_buffer.GetValue(value, 2);
    STF_ASSERT_EQ(buffer[2], value);
    data_buffer.GetValue(value, 3);
    STF_ASSERT_EQ(buffer[3], value);
}

STF_TEST(TestDataBuffer, GetValueInt8)
{
    std::int8_t buffer[4] = { 1, 2, -3, 4 };
    std::int8_t value;

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(4);
    data_buffer.SetValue(
        { reinterpret_cast<std::uint8_t *>(buffer), sizeof(buffer) },
        0);

    STF_ASSERT_EQ(1, data_buffer[0]);
    STF_ASSERT_EQ(2, data_buffer[1]);
    STF_ASSERT_EQ(0xfd, data_buffer[2]);
    STF_ASSERT_EQ(4, data_buffer[3]);

    // Read from the DataBuffer
    data_buffer.GetValue(value, 0);
    STF_ASSERT_EQ(buffer[0], value);
    data_buffer.GetValue(value, 1);
    STF_ASSERT_EQ(buffer[1], value);
    data_buffer.GetValue(value, 2);
    STF_ASSERT_EQ(buffer[2], value);
    data_buffer.GetValue(value, 3);
    STF_ASSERT_EQ(buffer[3], value);
}

STF_TEST(TestDataBuffer, GetValueUint16)
{
    std::uint16_t value;

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(64);
    value = 0, data_buffer.SetValue(value, 0);
    value = 1, data_buffer.SetValue(value, 2);
    value = 2, data_buffer.SetValue(value, 4);
    value = 3, data_buffer.SetValue(value, 6);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x01, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x02, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x03, data_buffer[7]);

    // Read from the buffer
    data_buffer.GetValue(value, 0);
    STF_ASSERT_EQ(0, value);
    data_buffer.GetValue(value, 2);
    STF_ASSERT_EQ(1, value);
    data_buffer.GetValue(value, 4);
    STF_ASSERT_EQ(2, value);
    data_buffer.GetValue(value, 6);
    STF_ASSERT_EQ(3, value);
}

STF_TEST(TestDataBuffer, GetValueInt16)
{
    std::int16_t value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value =  0, data_buffer.SetValue(value, 0);
    value =  1, data_buffer.SetValue(value, 2);
    value =  2, data_buffer.SetValue(value, 4);
    value =  3, data_buffer.SetValue(value, 6);
    value = -2, data_buffer.SetValue(value, 8);

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x01, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x02, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x03, data_buffer[7]);
    STF_ASSERT_EQ(0xFF, data_buffer[8]);
    STF_ASSERT_EQ(0xFE, data_buffer[9]);

    // Read from the buffer
    data_buffer.GetValue(value, 0);
    STF_ASSERT_EQ(0, value);
    data_buffer.GetValue(value, 2);
    STF_ASSERT_EQ(1, value);
    data_buffer.GetValue(value, 4);
    STF_ASSERT_EQ(2, value);
    data_buffer.GetValue(value, 6);
    STF_ASSERT_EQ(3, value);
    data_buffer.GetValue(value, 8);
    STF_ASSERT_EQ(-2, value);
}

STF_TEST(TestDataBuffer, GetValueUint32)
{
    std::uint32_t value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 0x01020304, data_buffer.SetValue(value, 0);
    value = 0x05060708, data_buffer.SetValue(value, 4);

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);

    // Read from the buffer
    data_buffer.GetValue(value, 0);
    STF_ASSERT_EQ(0x01020304, value);
    data_buffer.GetValue(value, 4);
    STF_ASSERT_EQ(0x05060708, value);
}

STF_TEST(TestDataBuffer, GetValueInt32)
{
    std::int32_t value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 0x01020304, data_buffer.SetValue(value, 0);
    value = -1858840968, data_buffer.SetValue(value, 4);

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x91, data_buffer[4]);
    STF_ASSERT_EQ(0x34, data_buffer[5]);
    STF_ASSERT_EQ(0x56, data_buffer[6]);
    STF_ASSERT_EQ(0x78, data_buffer[7]);

    // Read from the buffer
    data_buffer.GetValue(value, 0);
    STF_ASSERT_EQ(0x01020304, value);
    data_buffer.GetValue(value, 4);
    STF_ASSERT_EQ(-1858840968, value);
}

STF_TEST(TestDataBuffer, GetValueUint64)
{
    std::uint64_t value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 0x0102030405060708, data_buffer.SetValue(value, 0);
    value = 0x1112131415161718, data_buffer.SetValue(value, 8);

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);

    STF_ASSERT_EQ(0x11, data_buffer[8]);
    STF_ASSERT_EQ(0x12, data_buffer[9]);
    STF_ASSERT_EQ(0x13, data_buffer[10]);
    STF_ASSERT_EQ(0x14, data_buffer[11]);
    STF_ASSERT_EQ(0x15, data_buffer[12]);
    STF_ASSERT_EQ(0x16, data_buffer[13]);
    STF_ASSERT_EQ(0x17, data_buffer[14]);
    STF_ASSERT_EQ(0x18, data_buffer[15]);

    // Read from the buffer
    data_buffer.GetValue(value, 0);
    STF_ASSERT_EQ(0x0102030405060708, value);
    data_buffer.GetValue(value, 8);
    STF_ASSERT_EQ(0x1112131415161718, value);
}

STF_TEST(TestDataBuffer, GetValueInt64)
{
    std::int64_t value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 0x0102030405060708, data_buffer.SetValue(value, 0);
    value = -7993305411655166184, data_buffer.SetValue(value, 8);

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);

    STF_ASSERT_EQ(0x91, data_buffer[8]);
    STF_ASSERT_EQ(0x12, data_buffer[9]);
    STF_ASSERT_EQ(0x13, data_buffer[10]);
    STF_ASSERT_EQ(0x14, data_buffer[11]);
    STF_ASSERT_EQ(0x15, data_buffer[12]);
    STF_ASSERT_EQ(0x16, data_buffer[13]);
    STF_ASSERT_EQ(0x17, data_buffer[14]);
    STF_ASSERT_EQ(0x18, data_buffer[15]);

    // Read from the buffer
    data_buffer.GetValue(value, 0);
    STF_ASSERT_EQ(0x0102030405060708, value);
    data_buffer.GetValue(value, 8);
    STF_ASSERT_EQ(-7993305411655166184, value);
}

STF_TEST(TestDataBuffer, GetValueFloat)
{
    float value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 3.25f, data_buffer.SetValue(value, 0);
    value = -12.5f, data_buffer.SetValue(value, 4);

    STF_ASSERT_EQ(0x40, data_buffer[0]);
    STF_ASSERT_EQ(0x50, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0xc1, data_buffer[4]);
    STF_ASSERT_EQ(0x48, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x00, data_buffer[7]);

    // Read from the buffer
    data_buffer.GetValue(value, 0);
    STF_ASSERT_CLOSE(3.25f, value, 0.001f);
    data_buffer.GetValue(value, 4);
    STF_ASSERT_CLOSE(-12.5f, value, 0.001f);
}

STF_TEST(TestDataBuffer, GetValueDouble)
{
    double value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 3.25, data_buffer.SetValue(value, 0);
    value = -12.5, data_buffer.SetValue(value, 8);

    STF_ASSERT_EQ(0x40, data_buffer[0]);
    STF_ASSERT_EQ(0x0a, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x00, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x00, data_buffer[7]);
    STF_ASSERT_EQ(0xc0, data_buffer[8]);
    STF_ASSERT_EQ(0x29, data_buffer[9]);
    STF_ASSERT_EQ(0x00, data_buffer[10]);
    STF_ASSERT_EQ(0x00, data_buffer[11]);
    STF_ASSERT_EQ(0x00, data_buffer[12]);
    STF_ASSERT_EQ(0x00, data_buffer[13]);
    STF_ASSERT_EQ(0x00, data_buffer[14]);
    STF_ASSERT_EQ(0x00, data_buffer[15]);

    // Read from the buffer
    data_buffer.GetValue(value, 0);
    STF_ASSERT_CLOSE(3.25, value, 0.001);
    data_buffer.GetValue(value, 8);
    STF_ASSERT_CLOSE(-12.5, value, 0.001);
}

STF_TEST(TestDataBuffer, AppendValueArray)
{
    std::uint8_t buffer[4] = { 0x00, 0x01, 0x02, 0x03 };

    NetUtil::DataBuffer data_buffer(4);

    data_buffer.AppendValue(buffer);
    STF_ASSERT_EQ(4, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);
}

STF_TEST(TestDataBuffer, AppendValueVector)
{
    std::vector<uint8_t> data = { 0x00, 0x01, 0x02, 0x03 };

    NetUtil::DataBuffer data_buffer(4);

    data_buffer.AppendValue(data);
    STF_ASSERT_EQ(4, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);
}

STF_TEST(TestDataBuffer, AppendValueString)
{
    std::string data = { 0x00, 0x01, 0x02, 0x03, 04 };

    NetUtil::DataBuffer data_buffer(64);

    data_buffer.AppendValue(data);
    STF_ASSERT_EQ(5, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);
    STF_ASSERT_EQ(0x04, data_buffer[4]);
}

STF_TEST(TestDataBuffer, AppendValueUint8)
{
    std::uint8_t value;
    NetUtil::DataBuffer data_buffer(64);

    value = 0, data_buffer.AppendValue(value);
    value = 1, data_buffer.AppendValue(value);
    value = 2, data_buffer.AppendValue(value);
    value = 3, data_buffer.AppendValue(value);

    STF_ASSERT_EQ(4, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);
}

STF_TEST(TestDataBuffer, AppendValueInt8)
{
    std::int8_t value;
    NetUtil::DataBuffer data_buffer(64);

    value =  0, data_buffer.AppendValue(value);
    value =  1, data_buffer.AppendValue(value);
    value =  2, data_buffer.AppendValue(value);
    value =  3, data_buffer.AppendValue(value);
    value = -1, data_buffer.AppendValue(value);

    STF_ASSERT_EQ(5, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);
    STF_ASSERT_EQ(0xFF, data_buffer[4]);
}

STF_TEST(TestDataBuffer, AppendValueUint16)
{
    std::uint16_t value;

    NetUtil::DataBuffer data_buffer(64);
    value = 0, data_buffer.AppendValue(value);
    value = 1, data_buffer.AppendValue(value);
    value = 2, data_buffer.AppendValue(value);
    value = 3, data_buffer.AppendValue(value);

    STF_ASSERT_EQ(8, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x01, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x02, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x03, data_buffer[7]);
}

STF_TEST(TestDataBuffer, AppendValueInt16)
{
    std::int16_t value;

    NetUtil::DataBuffer data_buffer(64);
    value =  0, data_buffer.AppendValue(value);
    value =  1, data_buffer.AppendValue(value);
    value =  2, data_buffer.AppendValue(value);
    value =  3, data_buffer.AppendValue(value);
    value = -2, data_buffer.AppendValue(value);

    STF_ASSERT_EQ(10, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x01, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x02, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x03, data_buffer[7]);
    STF_ASSERT_EQ(0xFF, data_buffer[8]);
    STF_ASSERT_EQ(0xFE, data_buffer[9]);
}

STF_TEST(TestDataBuffer, AppendValueUint32)
{
    std::uint32_t value;

    NetUtil::DataBuffer data_buffer(64);
    value = 0x01020304, data_buffer.AppendValue(value);
    value = 0x05060708, data_buffer.AppendValue(value);

    STF_ASSERT_EQ(8, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);
}

STF_TEST(TestDataBuffer, AppendValueInt32)
{
    std::int32_t value;

    NetUtil::DataBuffer data_buffer(64);
    value = 0x01020304, data_buffer.AppendValue(value);
    value = -1858840968, data_buffer.AppendValue(value);

    STF_ASSERT_EQ(8, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x91, data_buffer[4]);
    STF_ASSERT_EQ(0x34, data_buffer[5]);
    STF_ASSERT_EQ(0x56, data_buffer[6]);
    STF_ASSERT_EQ(0x78, data_buffer[7]);
}

STF_TEST(TestDataBuffer, AppendValueUint64)
{
    std::uint64_t value;

    NetUtil::DataBuffer data_buffer(64);
    value = 0x0102030405060708, data_buffer.AppendValue(value);
    value = 0x1112131415161718, data_buffer.AppendValue(value);

    STF_ASSERT_EQ(16, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);

    STF_ASSERT_EQ(0x11, data_buffer[8]);
    STF_ASSERT_EQ(0x12, data_buffer[9]);
    STF_ASSERT_EQ(0x13, data_buffer[10]);
    STF_ASSERT_EQ(0x14, data_buffer[11]);
    STF_ASSERT_EQ(0x15, data_buffer[12]);
    STF_ASSERT_EQ(0x16, data_buffer[13]);
    STF_ASSERT_EQ(0x17, data_buffer[14]);
    STF_ASSERT_EQ(0x18, data_buffer[15]);
}

STF_TEST(TestDataBuffer, AppendValueInt64)
{
    std::int64_t value;

    NetUtil::DataBuffer data_buffer(64);
    value = 0x0102030405060708, data_buffer.AppendValue(value);
    value = -7993305411655166184, data_buffer.AppendValue(value);

    STF_ASSERT_EQ(16, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);

    STF_ASSERT_EQ(0x91, data_buffer[8]);
    STF_ASSERT_EQ(0x12, data_buffer[9]);
    STF_ASSERT_EQ(0x13, data_buffer[10]);
    STF_ASSERT_EQ(0x14, data_buffer[11]);
    STF_ASSERT_EQ(0x15, data_buffer[12]);
    STF_ASSERT_EQ(0x16, data_buffer[13]);
    STF_ASSERT_EQ(0x17, data_buffer[14]);
    STF_ASSERT_EQ(0x18, data_buffer[15]);
}

STF_TEST(TestDataBuffer, AppendValueFloat)
{
    float value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 3.25f, data_buffer.AppendValue(value);
    value = -12.5f, data_buffer.AppendValue(value);

    STF_ASSERT_EQ(8, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x40, data_buffer[0]);
    STF_ASSERT_EQ(0x50, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0xc1, data_buffer[4]);
    STF_ASSERT_EQ(0x48, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x00, data_buffer[7]);
}

STF_TEST(TestDataBuffer, AppendValueDouble)
{
    double value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 3.25, data_buffer.AppendValue(value);
    value = -12.5, data_buffer.AppendValue(value);

    STF_ASSERT_EQ(16, data_buffer.GetDataLength());

    STF_ASSERT_EQ(0x40, data_buffer[0]);
    STF_ASSERT_EQ(0x0a, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x00, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x00, data_buffer[7]);
    STF_ASSERT_EQ(0xc0, data_buffer[8]);
    STF_ASSERT_EQ(0x29, data_buffer[9]);
    STF_ASSERT_EQ(0x00, data_buffer[10]);
    STF_ASSERT_EQ(0x00, data_buffer[11]);
    STF_ASSERT_EQ(0x00, data_buffer[12]);
    STF_ASSERT_EQ(0x00, data_buffer[13]);
    STF_ASSERT_EQ(0x00, data_buffer[14]);
    STF_ASSERT_EQ(0x00, data_buffer[15]);
}

STF_TEST(TestDataBuffer, ReadValueArray)
{
    std::uint8_t buffer[4] = { 0x00, 0x01, 0x02, 0x03 };
    std::uint8_t buffer_read[4] = { 0xff, 0xff, 0xff, 0xff };

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(4);
    data_buffer.AppendValue(buffer);
    STF_ASSERT_EQ(4, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);

    // Read from the DataBuffer
    data_buffer.ReadValue(buffer_read);
    STF_ASSERT_EQ(4, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(buffer[0], buffer_read[0]);
    STF_ASSERT_EQ(buffer[1], buffer_read[1]);
    STF_ASSERT_EQ(buffer[2], buffer_read[2]);
    STF_ASSERT_EQ(buffer[3], buffer_read[3]);
}

STF_TEST(TestDataBuffer, ReadValueVector)
{
    std::vector<std::uint8_t> vector_write = { 0x00, 0x01, 0x02, 0x03 };
    std::vector<std::uint8_t> vector_read(4);

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(4);
    data_buffer.AppendValue(vector_write);
    STF_ASSERT_EQ(4, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);

    // Read from the DataBuffer
    data_buffer.ReadValue(vector_read);
    STF_ASSERT_EQ(4, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(vector_write[0], vector_read[0]);
    STF_ASSERT_EQ(vector_write[1], vector_read[1]);
    STF_ASSERT_EQ(vector_write[2], vector_read[2]);
    STF_ASSERT_EQ(vector_write[3], vector_read[3]);
}

STF_TEST(TestDataBuffer, ReadValueString)
{
    std::string string_write = { 0x00, 0x01, 0x02, 0x03 };
    std::string string_read(4, 0x7f);

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(4);
    data_buffer.AppendValue(string_write);
    STF_ASSERT_EQ(4, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);

    // Read from the DataBuffer
    data_buffer.ReadValue(string_read);
    STF_ASSERT_EQ(4, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(string_write[0], string_read[0]);
    STF_ASSERT_EQ(string_write[1], string_read[1]);
    STF_ASSERT_EQ(string_write[2], string_read[2]);
    STF_ASSERT_EQ(string_write[3], string_read[3]);
}

STF_TEST(TestDataBuffer, ReadValueUint8)
{
    std::uint8_t buffer[4] = { 0x00, 0x01, 0x02, 0x03 };
    std::uint8_t value;

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(4);
    data_buffer.AppendValue(buffer);
    STF_ASSERT_EQ(4, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x01, data_buffer[1]);
    STF_ASSERT_EQ(0x02, data_buffer[2]);
    STF_ASSERT_EQ(0x03, data_buffer[3]);

    // Read from the DataBuffer
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(buffer[0], value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(buffer[1], value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(buffer[2], value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(buffer[3], value);
    STF_ASSERT_EQ(4, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, ReadValueInt8)
{
    std::int8_t buffer[4] = { 1, 2, -3, 4 };
    std::int8_t value;

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(4);
    data_buffer.AppendValue(
        { reinterpret_cast<uint8_t *>(buffer), sizeof(buffer) });
    STF_ASSERT_EQ(4, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(1, data_buffer[0]);
    STF_ASSERT_EQ(2, data_buffer[1]);
    STF_ASSERT_EQ(0xfd, data_buffer[2]);
    STF_ASSERT_EQ(4, data_buffer[3]);

    // Read from the DataBuffer
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(buffer[0], value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(buffer[1], value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(buffer[2], value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(buffer[3], value);
    STF_ASSERT_EQ(4, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, ReadValueUint16)
{
    std::uint16_t value;

    // Populate the DataBuffer
    NetUtil::DataBuffer data_buffer(64);
    value = 0, data_buffer.AppendValue(value);
    value = 1, data_buffer.AppendValue(value);
    value = 2, data_buffer.AppendValue(value);
    value = 3, data_buffer.AppendValue(value);
    STF_ASSERT_EQ(8, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x01, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x02, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x03, data_buffer[7]);

    // Read from the buffer
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(0, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(1, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(2, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(3, value);
    STF_ASSERT_EQ(8, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, ReadValueInt16)
{
    std::int16_t value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value =  0, data_buffer.AppendValue(value);
    value =  1, data_buffer.AppendValue(value);
    value =  2, data_buffer.AppendValue(value);
    value =  3, data_buffer.AppendValue(value);
    value = -2, data_buffer.AppendValue(value);
    STF_ASSERT_EQ(10, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x00, data_buffer[0]);
    STF_ASSERT_EQ(0x00, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x01, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x02, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x03, data_buffer[7]);
    STF_ASSERT_EQ(0xFF, data_buffer[8]);
    STF_ASSERT_EQ(0xFE, data_buffer[9]);

    // Read from the buffer
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(0, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(1, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(2, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(3, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(-2, value);
    STF_ASSERT_EQ(10, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, ReadValueUint32)
{
    std::uint32_t value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 0x01020304, data_buffer.AppendValue(value);
    value = 0x05060708, data_buffer.AppendValue(value);
    STF_ASSERT_EQ(8, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);

    // Read from the buffer
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(0x01020304, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(0x05060708, value);
    STF_ASSERT_EQ(8, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, ReadValueInt32)
{
    std::int32_t value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 0x01020304, data_buffer.AppendValue(value);
    value = -1858840968, data_buffer.AppendValue(value);
    STF_ASSERT_EQ(8, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x91, data_buffer[4]);
    STF_ASSERT_EQ(0x34, data_buffer[5]);
    STF_ASSERT_EQ(0x56, data_buffer[6]);
    STF_ASSERT_EQ(0x78, data_buffer[7]);

    // Read from the buffer
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(0x01020304, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(-1858840968, value);
    STF_ASSERT_EQ(8, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, ReadValueUint64)
{
    std::uint64_t value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 0x0102030405060708, data_buffer.AppendValue(value);
    value = 0x1112131415161718, data_buffer.AppendValue(value);
    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);

    STF_ASSERT_EQ(0x11, data_buffer[8]);
    STF_ASSERT_EQ(0x12, data_buffer[9]);
    STF_ASSERT_EQ(0x13, data_buffer[10]);
    STF_ASSERT_EQ(0x14, data_buffer[11]);
    STF_ASSERT_EQ(0x15, data_buffer[12]);
    STF_ASSERT_EQ(0x16, data_buffer[13]);
    STF_ASSERT_EQ(0x17, data_buffer[14]);
    STF_ASSERT_EQ(0x18, data_buffer[15]);

    // Read from the buffer
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(0x0102030405060708, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(0x1112131415161718, value);
    STF_ASSERT_EQ(16, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, ReadValueInt64)
{
    std::int64_t value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 0x0102030405060708, data_buffer.AppendValue(value);
    value = -7993305411655166184, data_buffer.AppendValue(value);
    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x01, data_buffer[0]);
    STF_ASSERT_EQ(0x02, data_buffer[1]);
    STF_ASSERT_EQ(0x03, data_buffer[2]);
    STF_ASSERT_EQ(0x04, data_buffer[3]);
    STF_ASSERT_EQ(0x05, data_buffer[4]);
    STF_ASSERT_EQ(0x06, data_buffer[5]);
    STF_ASSERT_EQ(0x07, data_buffer[6]);
    STF_ASSERT_EQ(0x08, data_buffer[7]);

    STF_ASSERT_EQ(0x91, data_buffer[8]);
    STF_ASSERT_EQ(0x12, data_buffer[9]);
    STF_ASSERT_EQ(0x13, data_buffer[10]);
    STF_ASSERT_EQ(0x14, data_buffer[11]);
    STF_ASSERT_EQ(0x15, data_buffer[12]);
    STF_ASSERT_EQ(0x16, data_buffer[13]);
    STF_ASSERT_EQ(0x17, data_buffer[14]);
    STF_ASSERT_EQ(0x18, data_buffer[15]);

    // Read from the buffer
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(0x0102030405060708, value);
    data_buffer.ReadValue(value);
    STF_ASSERT_EQ(-7993305411655166184, value);
    STF_ASSERT_EQ(16, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, ReadValueFloat)
{
    float value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 3.25f, data_buffer.AppendValue(value);
    value = -12.5f, data_buffer.AppendValue(value);
    STF_ASSERT_EQ(8, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x40, data_buffer[0]);
    STF_ASSERT_EQ(0x50, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0xc1, data_buffer[4]);
    STF_ASSERT_EQ(0x48, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x00, data_buffer[7]);

    // Read from the buffer
    data_buffer.ReadValue(value);
    STF_ASSERT_CLOSE(3.25f, value, 0.001f);
    data_buffer.ReadValue(value);
    STF_ASSERT_CLOSE(-12.5f, value, 0.001f);
    STF_ASSERT_EQ(8, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, ReadValueDouble)
{
    double value;

    // Populate the data buffer
    NetUtil::DataBuffer data_buffer(64);
    value = 3.25, data_buffer.AppendValue(value);
    value = -12.5, data_buffer.AppendValue(value);
    STF_ASSERT_EQ(16, data_buffer.GetDataLength());
    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());

    STF_ASSERT_EQ(0x40, data_buffer[0]);
    STF_ASSERT_EQ(0x0a, data_buffer[1]);
    STF_ASSERT_EQ(0x00, data_buffer[2]);
    STF_ASSERT_EQ(0x00, data_buffer[3]);
    STF_ASSERT_EQ(0x00, data_buffer[4]);
    STF_ASSERT_EQ(0x00, data_buffer[5]);
    STF_ASSERT_EQ(0x00, data_buffer[6]);
    STF_ASSERT_EQ(0x00, data_buffer[7]);
    STF_ASSERT_EQ(0xc0, data_buffer[8]);
    STF_ASSERT_EQ(0x29, data_buffer[9]);
    STF_ASSERT_EQ(0x00, data_buffer[10]);
    STF_ASSERT_EQ(0x00, data_buffer[11]);
    STF_ASSERT_EQ(0x00, data_buffer[12]);
    STF_ASSERT_EQ(0x00, data_buffer[13]);
    STF_ASSERT_EQ(0x00, data_buffer[14]);
    STF_ASSERT_EQ(0x00, data_buffer[15]);

    // Read from the buffer
    data_buffer.ReadValue(value);
    STF_ASSERT_CLOSE(3.25, value, 0.001);
    data_buffer.ReadValue(value);
    STF_ASSERT_CLOSE(-12.5, value, 0.001);
    STF_ASSERT_EQ(16, data_buffer.GetReadPosition());
}

STF_TEST(TestDataBuffer, StreamingOperators)
{
    NetUtil::DataBuffer data_buffer(512);

    struct TestStruct
    {
        std::uint8_t i8;
        std::uint16_t i16;
        std::uint32_t i32;
        std::uint64_t i64;
        float f32;
        double f64;
        std::vector<std::uint8_t> v = { 0, 0, 0, 0 };
    };

    TestStruct original =
    {
        .i8 = 0x12,
        .i16 = 0x0003,
        .i32 = 0xdeadbeef,
        .i64 = 0x1112131415161718,
        .f32 = 3.25f,
        .f64 = -12.5,
        .v = { 0x10, 0x20, 0x30, 0x40 },
    };

    data_buffer << original.i8 << original.i16 << original.i32 << original.i64;
    data_buffer << original.f32 << original.f64 << original.v;

    STF_ASSERT_EQ(0x12, data_buffer[0]); // i8

    STF_ASSERT_EQ(0x00, data_buffer[1]); // i16
    STF_ASSERT_EQ(0x03, data_buffer[2]);

    STF_ASSERT_EQ(0xde, data_buffer[3]); // i32
    STF_ASSERT_EQ(0xad, data_buffer[4]);
    STF_ASSERT_EQ(0xbe, data_buffer[5]);
    STF_ASSERT_EQ(0xef, data_buffer[6]);

    STF_ASSERT_EQ(0x11, data_buffer[7]); // i64
    STF_ASSERT_EQ(0x12, data_buffer[8]);
    STF_ASSERT_EQ(0x13, data_buffer[9]);
    STF_ASSERT_EQ(0x14, data_buffer[10]);
    STF_ASSERT_EQ(0x15, data_buffer[11]);
    STF_ASSERT_EQ(0x16, data_buffer[12]);
    STF_ASSERT_EQ(0x17, data_buffer[13]);
    STF_ASSERT_EQ(0x18, data_buffer[14]);

    STF_ASSERT_EQ(0x40, data_buffer[15]); // f32
    STF_ASSERT_EQ(0x50, data_buffer[16]);
    STF_ASSERT_EQ(0x00, data_buffer[17]);
    STF_ASSERT_EQ(0x00, data_buffer[18]);

    STF_ASSERT_EQ(0xc0, data_buffer[19]); // f64
    STF_ASSERT_EQ(0x29, data_buffer[20]);
    STF_ASSERT_EQ(0x00, data_buffer[21]);
    STF_ASSERT_EQ(0x00, data_buffer[22]);
    STF_ASSERT_EQ(0x00, data_buffer[23]);
    STF_ASSERT_EQ(0x00, data_buffer[24]);
    STF_ASSERT_EQ(0x00, data_buffer[25]);
    STF_ASSERT_EQ(0x00, data_buffer[26]);

    STF_ASSERT_EQ(0x10, data_buffer[27]); // v
    STF_ASSERT_EQ(0x20, data_buffer[28]);
    STF_ASSERT_EQ(0x30, data_buffer[29]);
    STF_ASSERT_EQ(0x40, data_buffer[30]);

    // Now read from the buffer
    TestStruct output{};

    data_buffer >> output.i8 >> output.i16 >> output.i32 >> output.i64;
    data_buffer >> output.f32 >> output.f64 >> output.v;

    // Ensure all read values are as expected
    STF_ASSERT_EQ(original.i8, output.i8);
    STF_ASSERT_EQ(original.i16, output.i16);
    STF_ASSERT_EQ(original.i32, output.i32);
    STF_ASSERT_EQ(original.i64, output.i64);
    STF_ASSERT_EQ(original.f32, output.f32);
    STF_ASSERT_EQ(original.f64, output.f64);
    STF_ASSERT_EQ(original.v, output.v);
}

STF_TEST(TestDataBuffer, OutputStream1)
{
    NetUtil::DataBuffer buffer(128);
    std::string test_string = "This is a sample test string and a test of the "
                              "output stream operator";
    std::string expected_result =
R"(00000000: 00 01 02 03 54 68 69 73 20 69 73 20 61 20 73 61 :....This is a sa:
00000010: 6D 70 6C 65 20 74 65 73 74 20 73 74 72 69 6E 67 :mple test string:
00000020: 20 61 6E 64 20 61 20 74 65 73 74 20 6F 66 20 74 : and a test of t:
00000030: 68 65 20 6F 75 74 70 75 74 20 73 74 72 65 61 6D :he output stream:
00000040: 20 6F 70 65 72 61 74 6F 72                      : operator       :
)";

    // Append some non-printable values
    buffer.AppendValue(std::uint8_t(0));
    buffer.AppendValue(std::uint8_t(1));
    buffer.AppendValue(std::uint8_t(2));
    buffer.AppendValue(std::uint8_t(3));

    // Append the string to the buffer
    buffer.AppendValue(test_string);

    std::ostringstream oss;

    oss << buffer;

    std::string result = oss.str();

    STF_ASSERT_EQ(expected_result, result);
}

STF_TEST(TestDataBuffer, OutputStream2)
{
    NetUtil::DataBuffer buffer(128);
    std::string expected_result =
R"(00000000: FF FF 00 00 00 01                               :......          :
)";

    // Append some non-printable values
    buffer.AppendValue(std::uint16_t(-1));
    buffer.AppendValue(std::uint16_t(0));
    buffer.AppendValue(std::uint16_t(1));

    std::ostringstream oss;

    oss << buffer;

    std::string result = oss.str();

    STF_ASSERT_EQ(expected_result, result);
}

STF_TEST(TestDataBuffer, PassAsSpan)
{
    NetUtil::DataBuffer buffer(100);

    buffer << std::uint64_t(1);

    STF_ASSERT_EQ(8, buffer.GetDataLength());

    STF_ASSERT_EQ(8, SpanReceiver(buffer));
}

STF_TEST(TestDataBuffer, RangeForLoop)
{
    NetUtil::DataBuffer buffer(100);

    buffer << std::uint8_t(4);
    buffer << std::uint8_t(3);
    buffer << std::uint8_t(2);
    buffer << std::uint8_t(1);

    STF_ASSERT_EQ(4, buffer.GetDataLength());

    for (std::uint8_t e = 4; auto c : buffer) STF_ASSERT_EQ(e--, c);
}

STF_TEST(TestDataBuffer, ExplicitIterator)
{
    NetUtil::DataBuffer buffer(100);

    buffer << std::uint8_t(8);
    buffer << std::uint8_t(7);
    buffer << std::uint8_t(6);
    buffer << std::uint8_t(5);

    std::uint8_t e = 8;
    for (auto it = buffer.begin(); it != buffer.end(); it++)
    {
        STF_ASSERT_EQ(e--, *it);
    }
}

STF_TEST(TestDataBufer, StringStream)
{
    std::string hello_string = "hello";
    NetUtil::DataBuffer data_buffer(64);

    data_buffer << hello_string;

    STF_ASSERT_EQ(5, data_buffer.GetDataLength());
}

STF_TEST(TestDataBufer, ChainedStream)
{
    std::uint32_t cafe_babe = 0xcafebabe;
    std::string hello_string = "hello";
    NetUtil::DataBuffer data_buffer(64);

    data_buffer << hello_string << cafe_babe;

    STF_ASSERT_EQ(0, data_buffer.GetReadPosition());
    STF_ASSERT_EQ(9, data_buffer.GetDataLength());

    STF_ASSERT_EQ('h', data_buffer[0]);
    STF_ASSERT_EQ('e', data_buffer[1]);
    STF_ASSERT_EQ('l', data_buffer[2]);
    STF_ASSERT_EQ('l', data_buffer[3]);
    STF_ASSERT_EQ('o', data_buffer[4]);
    STF_ASSERT_EQ(0xca, data_buffer[5]);
    STF_ASSERT_EQ(0xfe, data_buffer[6]);
    STF_ASSERT_EQ(0xba, data_buffer[7]);
    STF_ASSERT_EQ(0xbe, data_buffer[8]);

    // Read the values back
    std::uint8_t hello_string_read[5];
    std::uint32_t cafe_babe_read;

    data_buffer >> hello_string_read >> cafe_babe_read;

}
