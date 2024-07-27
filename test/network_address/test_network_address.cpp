/*
 *  test_network_address.cpp
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This file implements unit tests for the NetworkAddress object.
 *
 *  Portability Issues:
 *      None.
 */

#include <map>
#include <unordered_map>
#include <sstream>
#include <terra/netutil/network_address.h>
#include <terra/stf/stf.h>

using namespace Terra;

STF_TEST(NetworkAddress, EmptyAddress)
{
    NetUtil::NetworkAddress address;

    STF_ASSERT_TRUE(address.Empty());

    STF_ASSERT_FALSE(address);

    if (address) STF_ASSERT_TRUE(false);
}

STF_TEST(NetworkAddress, NonEmptyAddress)
{
    NetUtil::NetworkAddress address("127.0.0.1");

    STF_ASSERT_FALSE(address.Empty());

    STF_ASSERT_TRUE(address);

    if (!address) STF_ASSERT_TRUE(false);
}

STF_TEST(NetworkAddress, CopyAddress1)
{
    NetUtil::NetworkAddress address("127.0.0.1");

    NetUtil::NetworkAddress address2(address);

    STF_ASSERT_EQ(address, address2);
}

STF_TEST(NetworkAddress, CopyAddress2)
{
    NetUtil::NetworkAddress address("127.0.0.1");

    NetUtil::NetworkAddress address2 = address;

    STF_ASSERT_EQ(address, address2);
}

STF_TEST(NetworkAddress, MoveAddress1)
{
    NetUtil::NetworkAddress address("127.0.0.1");
    NetUtil::NetworkAddress address2 = address;
    NetUtil::NetworkAddress address3(std::move(address));

    STF_ASSERT_EQ(address2, address3);
}

STF_TEST(NetworkAddress, MoveAddress2)
{
    NetUtil::NetworkAddress address("127.0.0.1");
    NetUtil::NetworkAddress address2 = address;
    NetUtil::NetworkAddress address3 = std::move(address);

    STF_ASSERT_EQ(address2, address3);
}

STF_TEST(NetworkAddress, AddressTypeString1)
{
    NetUtil::NetworkAddress address("127.0.0.1", 1234);

    STF_ASSERT_EQ(std::string("IPv4"), address.GetAddressTypeString());
}

STF_TEST(NetworkAddress, AddressTypeString2)
{
    NetUtil::NetworkAddress address("::1", 1234);

    STF_ASSERT_EQ(std::string("IPv6"), address.GetAddressTypeString());
}

STF_TEST(NetworkAddress, AddressAndPort1)
{
    NetUtil::NetworkAddress address("127.0.0.1", 1234);

    std::string s = address.GetAddress();

    STF_ASSERT_EQ(std::string("127.0.0.1"), s);
    STF_ASSERT_EQ(1234, address.GetPort());
}

STF_TEST(NetworkAddress, AddressAndPort2)
{
    NetUtil::NetworkAddress address("::1", 1234);

    STF_ASSERT_EQ(NetUtil::NetworkAddressType::IPv6, address.GetAddressType());

    std::string s = address.GetAddress();

    STF_ASSERT_EQ(std::string("::1"), s);
    STF_ASSERT_EQ(1234, address.GetPort());
}

STF_TEST(NetworkAddress, AddressAndPort3)
{
    NetUtil::NetworkAddress address("fd88:0::1", 1234);

    STF_ASSERT_EQ(NetUtil::NetworkAddressType::IPv6, address.GetAddressType());

    std::string s = address.GetAddress();

    // Verify normative format produced
    STF_ASSERT_EQ(std::string("fd88::1"), s);
    STF_ASSERT_EQ(1234, address.GetPort());
}

STF_TEST(NetworkAddress, MapStorage)
{
    std::map<NetUtil::NetworkAddress, unsigned> address_map;

    NetUtil::NetworkAddress address1("127.0.0.1");
    address_map[address1] = 1;

    NetUtil::NetworkAddress address2("::1");
    address_map[address2] = 2;

    STF_ASSERT_EQ(2, address_map.size());
}

STF_TEST(NetworkAddress, UnorderedMapStorage)
{
    std::unordered_map<NetUtil::NetworkAddress,
                       unsigned,
                       NetUtil::NetworkAddressHash> address_map;

    NetUtil::NetworkAddress address1("127.0.0.1");
    address_map[address1] = 1;

    NetUtil::NetworkAddress address2("::1");
    address_map[address2] = 2;

    STF_ASSERT_EQ(2, address_map.size());
}

STF_TEST(NetworkAddress, FriendlyAddressString1)
{
    std::ostringstream oss;

    NetUtil::NetworkAddress address1("127.0.0.1");

    oss << address1;

    STF_ASSERT_EQ(std::string("127.0.0.1"), oss.str());
}

STF_TEST(NetworkAddress, FriendlyAddressString2)
{
    std::ostringstream oss;

    NetUtil::NetworkAddress address1("127.0.0.1", 1720);

    oss << address1;

    STF_ASSERT_EQ(std::string("127.0.0.1:1720"), oss.str());
}

STF_TEST(NetworkAddress, FriendlyAddressString3)
{
    std::ostringstream oss;

    NetUtil::NetworkAddress address1("fd88:0:0::beef");

    oss << address1;

    STF_ASSERT_EQ(std::string("[fd88::beef]"), oss.str());
}

STF_TEST(NetworkAddress, FriendlyAddressString4)
{
    std::ostringstream oss;

    NetUtil::NetworkAddress address1("fd88:0:0::beef", 1720);

    oss << address1;

    STF_ASSERT_EQ(std::string("[fd88::beef]:1720"), oss.str());
}

STF_TEST(NetworkAddress, FriendlyAddressType)
{
    std::ostringstream oss;

    NetUtil::NetworkAddress address1("::1");

    oss << address1.GetAddressType();

    STF_ASSERT_EQ(std::string("IPv6"), oss.str());
}
