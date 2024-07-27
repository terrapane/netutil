/*
 *  network_address.h
 *
 *  Copyright (C) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This defines an object type called NetworkAddress that simplifies
 *      use of IPv4 and IPv6 addresses.  It will keep track of both a given
 *      IP address and port number.  One may elect to not give a port number,
 *      in which case zero is assigned and interpreted as not having a port
 *      value.
 *
 *  Portability Issues:
 *      None.
 */

#pragma once

#ifdef _WIN32
#include <Winsock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <ostream>
#include <string>
#include <cstdint>
#include <climits>

namespace Terra::NetUtil
{

// Define the supported network types
enum class NetworkAddressType
{
    Unknown = 0,
    IPv4 = 1,
    IPv6 = 2
};

// Define the NetworkAddress object
class NetworkAddress
{
    public:
        NetworkAddress();
        NetworkAddress(const std::string &address, std::uint16_t port = 0);
        NetworkAddress(const struct sockaddr *address,
                       socklen_t address_length);
        NetworkAddress(const struct sockaddr_storage *address,
                       socklen_t address_length);
        NetworkAddress(const NetworkAddress &other) noexcept;
        NetworkAddress(const NetworkAddress &&other) noexcept;
        ~NetworkAddress() = default;

        NetworkAddress& operator=(const NetworkAddress& other) noexcept;
        NetworkAddress& operator=(NetworkAddress&& other) noexcept;

        bool AssignAddress(const std::string &address, std::uint16_t port = 0);
        bool AssignAddress(const struct sockaddr *address,
                           socklen_t address_length);
        bool AssignAddress(const struct sockaddr_storage *address,
                           socklen_t address_length);
        std::string GetAddress() const;

        sockaddr_storage* GetAddressStorage();
        const sockaddr_storage* GetAddressStorage() const;
        socklen_t GetAddressStorageSize() const;

        bool AssignPort(std::uint16_t port);
        std::uint16_t GetPort() const;

        NetworkAddressType GetAddressType() const;
        static std::string GetAddressTypeString(NetworkAddressType type);
        std::string GetAddressTypeString();

        void ClearAddress();
        bool Empty() const;

        bool operator==(const NetworkAddress &other) const;
        bool operator!=(const NetworkAddress &other) const;
        explicit operator bool() const;
        bool operator<(const NetworkAddress &other) const;
        bool operator>(const NetworkAddress &other) const;

    protected:
        union
        {
            struct sockaddr         sa;
            struct sockaddr_in      sa4;
            struct sockaddr_in6     sa6;
            struct sockaddr_storage ss;
        } address_storage;
};

// Hash object to facilitate use of std::unordered_map
struct NetworkAddressHash
{
    std::size_t operator()(const NetworkAddress &address) const noexcept
    {
        std::size_t hash = 0;
        constexpr std::size_t hash_bits = sizeof(hash) * CHAR_BIT;

        const std::uint8_t *p = reinterpret_cast<const std::uint8_t *>(
                                                address.GetAddressStorage());
        const std::uint8_t *q = p + address.GetAddressStorageSize();

        while (p < q)
        {
            // XOR least-significant octet of hash
            hash ^= static_cast<std::size_t>(*p++);

            // Rotate the hash eight bits to the left
            hash = ((hash << 8) | (hash >> (hash_bits - 8)));
        }

        return hash;
    }
};

// Streaming operators for human-friendly output
std::ostream &operator<<(std::ostream &o, const NetworkAddress &address);
std::ostream &operator<<(std::ostream &o, const NetworkAddressType &type);

} // namespace NetUtil
