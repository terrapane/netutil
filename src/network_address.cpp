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
 *      This file implements the object type NetworkAddress that simplifies
 *      use of IPv4 and IPv6 addresses.  It will keep track of both a given
 *      IP address and port number.  One may elect to not give a port number,
 *      in which case zero is assigned and interpreted as not having a port
 *      value.
 *
 *  Portability Issues:
 *      None.
 */

#include <cstring>
#include <array>
#ifndef _WIN32
#include <arpa/inet.h>
#endif
#include <terra/netutil/network_address.h>

namespace Terra::NetUtil
{

/*
 *  NetworkAddress::NetworkAddress()
 *
 *  Description:
 *      Creates a default-initialized NetworkAddress object.
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
NetworkAddress::NetworkAddress() : address_storage{}
{
    // Nothing to do
}

/*
 *  NetworkAddress::NetworkAddress()
 *
 *  Description:
 *      Creates a NetworkAddress object having the given IP address and port.
 *
 *  Parameters:
 *      address [in]
 *          The IP address in textual format to assign to this object.
 *
 *      port [in]
 *          The port number to assign.  A value of zero is treated as not
 *          having a port assignment when printing the address.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      If the given IP address is invalid, assignment may fail.  One may
 *      check the object's assignment by calling Empty() or using the bool
 *      operator.
 */
NetworkAddress::NetworkAddress(const std::string &address, std::uint16_t port) :
    address_storage{}
{
    AssignAddress(address, port);
}

/*
 *  NetworkAddress::NetworkAddress()
 *
 *  Description:
 *      Creates a NetworkAddress object having the given IP address and port.
 *
 *  Parameters:
 *      address [in]
 *          The address stores in a sockaddr structure to assign.
 *
 *      length [in]
 *          The length of the sockaddr structure.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
NetworkAddress::NetworkAddress(const struct sockaddr *address,
                               socklen_t address_length) : address_storage{}
{
    AssignAddress(address, address_length);
}

/*
 *  NetworkAddress::NetworkAddress()
 *
 *  Description:
 *      Creates a NetworkAddress object having the given IP address and port.
 *
 *  Parameters:
 *      address [in]
 *          The address stored in a sockaddr_storage structure to assign.
 *
 *      length [in]
 *          The length of the sockaddr_storage structure.  This might be shorter
 *          than a sockaddr_storage since it might be a sockaddr, for example,
 *          cast to the sockaddr_storage type.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
NetworkAddress::NetworkAddress(const struct sockaddr_storage *address,
                               socklen_t address_length) : address_storage{}
{
    AssignAddress(address, address_length);
}

/*
 *  NetworkAddress::NetworkAddress()
 *
 *  Description:
 *      Copy constructor for the NetworkAddress object.
 *
 *  Parameters:
 *      other [in]
 *          The other NetworkAddress from which to copy information.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
NetworkAddress::NetworkAddress(const NetworkAddress &other) noexcept :
    address_storage{}
{
    // Copy the address data from the other object
    std::memcpy(&address_storage.ss,
                &other.address_storage,
                sizeof(address_storage.ss));
}

/*
 *  NetworkAddress::NetworkAddress()
 *
 *  Description:
 *      Move constructor for the NetworkAddress object.  This object cannot
 *      really move, so it just copies.
 *
 *  Parameters:
 *      other [in]
 *          The other NetworkAddress from which to move information.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
NetworkAddress::NetworkAddress(const NetworkAddress &&other) noexcept :
    address_storage{}
{
    // Copy the address data from the other object
    std::memcpy(&address_storage.ss,
                &other.address_storage,
                sizeof(address_storage.ss));
}

/*
 *  NetworkAddress::NetworkAddress()
 *
 *  Description:
 *      Copy assignment operator for the NetworkAddress object.
 *
 *  Parameters:
 *      other [in]
 *          The other NetworkAddress from which to copy information.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
NetworkAddress& NetworkAddress::operator=(const NetworkAddress& other) noexcept
{
    // Copy the address data from the other object
    std::memcpy(&address_storage.ss,
                &other.address_storage,
                sizeof(address_storage.ss));

    return *this;
}

/*
 *  NetworkAddress::NetworkAddress()
 *
 *  Description:
 *      Move assignment for the NetworkAddress object.
 *
 *  Parameters:
 *      other [in]
 *          The other NetworkAddress from which to copy information.  This
 *          object cannot really move, so it just copies.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:
 *      None.
 */
NetworkAddress& NetworkAddress::operator=(NetworkAddress&& other) noexcept
{
    // Copy the address data from the other object
    std::memcpy(&address_storage.ss,
                &other.address_storage,
                sizeof(address_storage.ss));

    return *this;
}

/*
 *  NetworkAddress::AssignAddress()
 *
 *  Description:
 *      Assigns the given IP address and port to the NetworkAddress object.
 *
 *  Parameters:
 *      address [in]
 *          The IP address in textual format to assign to this object.
 *
 *      port [in]
 *          The port number to assign.  A value of zero is treated as not
 *          having a port assignment when printing the address.
 *
 *  Returns:
 *      True if the address was assigned or false if it failed.
 *
 *  Comments:
 *      None.
 */
bool NetworkAddress::AssignAddress(const std::string &address,
                                   std::uint16_t port)
{
    int result{};

    // Wipe the currently stored address
    ClearAddress();

    // Assume the string is IPv4
    result = inet_pton(AF_INET, address.c_str(), &address_storage.sa4.sin_addr);

    // If successful, set the address family and port
    if (result == 1)
    {
        address_storage.sa4.sin_family = AF_INET;
        address_storage.sa4.sin_port = htons(port);
        return true;
    }

    // Assume this is an IPv6 address
    result = inet_pton(AF_INET6,
                       address.c_str(),
                       &address_storage.sa6.sin6_addr);

    // If successful, set the address family and port
    if (result == 1)
    {
        address_storage.sa6.sin6_family = AF_INET6;
        address_storage.sa6.sin6_port = htons(port);
        return true;
    }

    // The address could not be converted, so return an error
    return false;
}

/*
 *  NetworkAddress::AssignAddress()
 *
 *  Description:
 *      Assigns the given sockaddr to this NetworkAddress object.
 *
 *  Parameters:
 *      address [in]
 *          The address stores in a sockaddr structure to assign.
 *
 *      length [in]
 *          The length of the sockaddr structure.
 *
 *  Returns:
 *      True if the address was assigned or false if it failed.
 *
 *  Comments:
 *      None.
 */
bool NetworkAddress::AssignAddress(const struct sockaddr *address,
                                   socklen_t address_length)
{
    return AssignAddress(
        reinterpret_cast<const struct sockaddr_storage *>(address),
        address_length);
}

/*
 *  NetworkAddress::AssignAddress()
 *
 *  Description:
 *      Creates a NetworkAddress object having the given IP address and port.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      True if the address was assigned or false if it failed.
 *
 *  Comments:
 *      None.
 */
bool NetworkAddress::AssignAddress(const struct sockaddr_storage *address,
                                   socklen_t address_length)
{
    // Clear any current address information
    ClearAddress();

    // Ensure the address length is within bounds
    if ((address_length == 0) || (address_length > sizeof(address_storage.ss)))
    {
        return false;
    }

    // Copy the address data
    std::memcpy(&address_storage.ss, address, address_length);

    return true;
}

/*
 *  NetworkAddress::GetAddress()
 *
 *  Description:
 *      Returns the assigned address in text form or an empty string if
 *      the address is unassigned or contains an unknown address type. If
 *      the conversion of the address to a string fails for any reason,
 *      the result will also be an empty string.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The assigned IP address in text form or an empty string if there
 *      was an error.
 *
 *  Comments:
 *      None.
 */
std::string NetworkAddress::GetAddress() const
{
    std::array<char, INET6_ADDRSTRLEN> string_storage{};
    bool result = false;

    // Convert from binary to string form depending on the address type
    switch (address_storage.ss.ss_family)
    {
        case AF_INET:
            if (inet_ntop(AF_INET,
                          &address_storage.sa4.sin_addr,
                          string_storage.data(),
                          string_storage.size()) != nullptr)
            {
                result = true;
            }
            break;

        case AF_INET6:
            if (inet_ntop(AF_INET6,
                          &address_storage.sa6.sin6_addr,
                          string_storage.data(),
                          string_storage.size()) != nullptr)
            {
                result = true;
            }
            break;

        default:
            // Unknown or unspecified address type
            result = false;
            break;
    }

    // If unable to produce a string, return an empty one
    if (!result) return {};

    return {string_storage.data()};
}

/*
 *  NetworkAddress::GetAddressStorage()
 *
 *  Description:
 *      This returns a mutable pointer to the underlying sockaddr_storage
 *      structure.  This is useful when passing this to other network functions
 *      that wish to populate the structure.  Before populating this externally,
 *      one should first call Clear() if an address had been stored previously
 *      to zero out the structure.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The address of the underlying sockaddr_storage structure.
 *
 *  Comments:
 *      None.
 */
sockaddr_storage *NetworkAddress::GetAddressStorage()
{
    return &address_storage.ss;
}

/*
 *  NetworkAddress::GetAddressStorage()
 *
 *  Description:
 *      This returns a const pointer to the underlying sockaddr_storage
 *      structure.  This is useful when passing this to other network functions
 *      that wish to consume this value.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The address of the underlying sockaddr_storage structure.
 *
 *  Comments:
 *      None.
 */
const sockaddr_storage *NetworkAddress::GetAddressStorage() const
{
    return &address_storage.ss;
}

/*
 *  NetworkAddress::GetAddressStorageSize()
 *
 *  Description:
 *      This returns the size of the sockaddr_storage.  If an address is
 *      assigned, this will be the size of the corresponding storage structure
 *      (e.g., sockaddr_in vs. sockaddr_in6).  Otherwise, this function will
 *      return the sizeof(sockaddr_storage).
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The size of the sockaddr_storage or part thereof based on the address
 *      family currently assigned.
 *
 *  Comments:
 *      None.
 */
socklen_t NetworkAddress::GetAddressStorageSize() const
{
    socklen_t size = sizeof(address_storage);

    // Size is based on the assigned address family
    switch (address_storage.ss.ss_family)
    {
        case AF_INET:
            size = sizeof(sockaddr_in);
            break;

        case AF_INET6:
            size = sizeof(sockaddr_in6);
            break;

        default:
            // Nothing to do
            break;
    }

    return size;
}

/*
 *  NetworkAddress::AssignPort()
 *
 *  Description:
 *      This will assign the given port number to the NetworkAddress object.
 *      This function assumes that a valid address is already assigned.
 *      If not, this function will return false.
 *
 *  Parameters:
 *      port [in]
 *          Port number to assign in the range of 0..65535.
 *
 *  Returns:
 *      True if successful, false if not.
 *
 *  Comments:
 *      None.
 */
bool NetworkAddress::AssignPort(std::uint16_t port)
{
    bool result = false;

    // Assign the port based on the address family
    switch (address_storage.ss.ss_family)
    {
        case AF_INET:
            address_storage.sa4.sin_port = htons(port);
            result = true;
            break;

        case AF_INET6:
            address_storage.sa6.sin6_port = htons(port);
            result = true;
            break;

        default:
            // Unknown address family
            break;
    }

    return result;
}

/*
 *  NetworkAddress::AssignPort()
 *
 *  Description:
 *      Returns the port assigned to the NetworkAddress object.  If the
 *      object is not assigned an address, the value returned will be 0.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      The assigned port value or zero if either not assigned or if the
 *      address family is unknown.
 *
 *  Comments:
 *      None.
 */
std::uint16_t NetworkAddress::GetPort() const
{
    std::uint16_t port = 0;

    switch (address_storage.ss.ss_family)
    {
        case AF_INET:
            port = ntohs(address_storage.sa4.sin_port);
            break;

        case AF_INET6:
            port = ntohs(address_storage.sa6.sin6_port);
            break;

        default:
            // Unknown address family
            break;
    }

    return port;
}

/*
 *  NetworkAddress::GetAddressType()
 *
 *  Description:
 *      Returns assigned network address type of None unassigned or if it is
 *      indeterminate.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      True if successful, false if not.
 *
 *  Comments:
 *      None.
 */
NetworkAddressType NetworkAddress::GetAddressType() const
{
    NetworkAddressType type = NetworkAddressType::Unknown;

    switch (address_storage.ss.ss_family)
    {
        case AF_INET:
            type = NetworkAddressType::IPv4;
            break;

        case AF_INET6:
            type = NetworkAddressType::IPv6;
            break;

        default:
            // Unknown address family
            break;
    }

    return type;
}

/*
 *  NetworkAddress::GetAddressTypeString()
 *
 *  Description:
 *      Converts the network address type to a human-readable string.
 *
 *  Parameters:
 *      type [in]
 *          The address type to be converted into a string.
 *
 *  Returns:
 *      A string for the specified address type.
 *
 *  Comments:
 *      None.
 */
std::string NetworkAddress::GetAddressTypeString(NetworkAddressType type)
{
    std::string result;

    switch (type)
    {
        case NetworkAddressType::IPv4:
            result = "IPv4";
            break;
        case NetworkAddressType::IPv6:
            result = "IPv6";
            break;
        default:
            result = "Unknown";
            break;
    }

    return result;
}

/*
 *  NetworkAddress::GetAddressTypeString()
 *
 *  Description:
 *      Converts the network address type to a human-readable string.
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
std::string NetworkAddress::GetAddressTypeString()
{
    return GetAddressTypeString(GetAddressType());
}

/*
 *  NetworkAddress::ClearAddress()
 *
 *  Description:
 *      This will clear the currently stored IP address and port.
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
void NetworkAddress::ClearAddress()
{
    // Wipe the currently stored address
    std::memset(&address_storage, 0, sizeof(address_storage));
}

/*
 *  NetworkAddress::Empty()
 *
 *  Description:
 *      Returns true if the object has an assigned address.  Specifically,
 *      it checks to see if the address family is AF_UNSPEC and returns
 *      true if that's the case.  If one successfully assigned a valid IPv4
 *      or IPv6 value, this would return false.
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
bool NetworkAddress::Empty() const
{
    return (address_storage.ss.ss_family == AF_UNSPEC);
}

/*
 *  NetworkAddress::operator==()
 *
 *  Description:
 *      Checks to see if this and the other NetworkAddress are equal.
 *      This only compares the IP address and port values.  Values like
 *      IPv6 flow labels or scope IDs are not compared.
 *
 *  Parameters:
 *      other [in]
 *          The other NetworkAddress object against which to compare.
 *
 *  Returns:
 *      True if equal, false if not.
 *
 *  Comments:
 *      None.
 */
bool NetworkAddress::operator==(const NetworkAddress &other) const
{
    bool result = false;

    // If the address families are not equal, return false
    if (address_storage.ss.ss_family != other.address_storage.ss.ss_family)
    {
        return result;
    }

    // Compare address and port with respect to the address family
    switch (address_storage.ss.ss_family)
    {
        case AF_UNSPEC:
            // Nothing more to compare
            break;

        case AF_INET:
            // Compare addresses and port components
            if (((address_storage.sa4.sin_addr.s_addr ==
                  other.address_storage.sa4.sin_addr.s_addr)) ||
                ((address_storage.sa4.sin_port ==
                  other.address_storage.sa4.sin_port)))
            {
                result = true;
            }
            break;

        case AF_INET6:
            // Compare addresses and port components
            if ((!memcmp(&address_storage.sa6.sin6_addr.s6_addr,
                         &other.address_storage.sa6.sin6_addr.s6_addr,
                         sizeof(address_storage.sa6.sin6_addr.s6_addr))) &&
                (address_storage.sa6.sin6_port ==
                 other.address_storage.sa6.sin6_port))
            {
                result = true;
            }
            break;

        default:
            // Cannot compare unknown address types, so assume false
            break;
    }

    return result;
}

/*
 *  NetworkAddress::operator!=()
 *
 *  Description:
 *      Checks to see if this and the other NetworkAddress are NOT equal.
 *      This only compares the IP address and port values.  Values like
 *      IPv6 flow labels or scope IDs are not compared.
 *
 *  Parameters:
 *      other [in]
 *          The other NetworkAddress object against which to compare.
 *
 *  Returns:
 *      True if not equal, false if equal.
 *
 *  Comments:
 *      None.
 */
bool NetworkAddress::operator!=(const NetworkAddress &other) const
{
    return !(*this == other);
}

/*
 *  NetworkAddress::operator bool()
 *
 *  Description:
 *      Returns true if the object has an assigned address.  Specifically,
 *      it checks to see if the address family is AF_UNSPEC and returns
 *      true if that's the case.  If one successfully assigned a valid IPv4
 *      or IPv6 value, this would return false.
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
NetworkAddress::operator bool() const
{
    return (address_storage.ss.ss_family != AF_UNSPEC);
}

/*
 *  NetworkAddress::operator<()
 *
 *  Description:
 *      Test to see if one address is "less" than another.  The meaning is
 *      a little ambiguous, but the intent is to allow addresses to be used
 *      with objects that require consistent ordering (e.g., std::map).
 *
 *  Parameters:
 *      other [in]
 *          The other object with which to compare.
 *
 *  Returns:
 *      True if less than, false if not less than.
 *
 *  Comments:
 *      None.
 */
bool NetworkAddress::operator<(const NetworkAddress &other) const
{
    int result{};

    // If the address families are different, sort by address family
    if (address_storage.ss.ss_family != other.address_storage.ss.ss_family)
    {
        // Sort based on the address family value defined by the system
        return (address_storage.ss.ss_family <
                other.address_storage.ss.ss_family);
    }

    // Compare addresses and ports with respect to the address family
    switch (address_storage.ss.ss_family)
    {
        case AF_INET:
            // Is this address less than the other?
            if (address_storage.sa4.sin_addr.s_addr <
                other.address_storage.sa4.sin_addr.s_addr)
            {
                return true;
            }

            // Is this address greater than the other?
            if (address_storage.sa4.sin_addr.s_addr >
                other.address_storage.sa4.sin_addr.s_addr)
            {
                return false;
            }

            // Since the IPv4 addresses are equal, compare ports
            return (address_storage.sa4.sin_port <
                    other.address_storage.sa4.sin_port);
            break;

        case AF_INET6:
            result = std::memcmp(&address_storage.sa6.sin6_addr.s6_addr,
                                 &other.address_storage.sa6.sin6_addr.s6_addr,
                                 sizeof(address_storage.sa6.sin6_addr.s6_addr));

            // Is this address less than the other?
            if (result < 0) return true;

            // Is this address greater than the other?
            if (result > 0) return false;

            // Since the IPv6 addresses are equal, compare ports
            return (address_storage.sa6.sin6_port <
                    other.address_storage.sa6.sin6_port);
            break;

        default:
            // Unspecified addresses are considered equal
            break;
    }

    return false;
}

/*
 *  NetworkAddress::operator<()
 *
 *  Description:
 *      Test to see if one address is "greater" than another.  The meaning is
 *      a little ambiguous, but the intent is to allow addresses to be used
 *      with objects that require consistent ordering (e.g., std::map).
 *
 *  Parameters:
 *      other [in]
 *          The other object with which to compare.
 *
 *  Returns:
 *      True if less than, false if not less than.
 *
 *  Comments:
 *      None.
 */
bool NetworkAddress::operator>(const NetworkAddress &other) const
{
    int result{};

    // If the address families are different, sort by address family
    if (address_storage.ss.ss_family != other.address_storage.ss.ss_family)
    {
        // Sort based on the address family value defined by the system
        return (address_storage.ss.ss_family >
                other.address_storage.ss.ss_family);
    }

    // Compare addresses and ports with respect to the address family
    switch (address_storage.ss.ss_family)
    {
        case AF_INET:
            // Is this address less than the other?
            if (address_storage.sa4.sin_addr.s_addr >
                other.address_storage.sa4.sin_addr.s_addr)
            {
                return true;
            }

            // Is this address greater than the other?
            if (address_storage.sa4.sin_addr.s_addr <
                other.address_storage.sa4.sin_addr.s_addr)
            {
                return false;
            }

            // Since the IPv4 addresses are equal, compare ports
            return (address_storage.sa4.sin_port >
                    other.address_storage.sa4.sin_port);
            break;

        case AF_INET6:
            result = std::memcmp(&address_storage.sa6.sin6_addr.s6_addr,
                                 &other.address_storage.sa6.sin6_addr.s6_addr,
                                 sizeof(address_storage.sa6.sin6_addr.s6_addr));

            // Is this address less than the other?
            if (result > 0) return true;

            // Is this address greater than the other?
            if (result < 0) return false;

            // Since the IPv6 addresses are equal, compare ports
            return (address_storage.sa6.sin6_port >
                    other.address_storage.sa6.sin6_port);
            break;

        default:
            // Unspecified addresses are considered equal
            break;
    }

    return false;
}

/*
 *  operator<<() for NetworkAddress
 *
 *  Description:
 *      Streaming operator to produce a human-friendly version of the
 *      NetworkAddress object value.  The format of this streaming operator
 *      is similar to "127.0.0.1:1720" and "[fd88::1]:1720".  The colon and
 *      port value will not be printed if the port number evaluates to zero.
 *
 *  Parameters:
 *      o [out]
 *          Output stream.
 *
 *      address [in]
 *          The network address to produce.
 *
 *  Returns:
 *      The output stream reference.
 *
 *  Comments:
 *      None.
 */
std::ostream &operator<<(std::ostream &o, const NetworkAddress &address)
{
    // Only print something if the address is assigned
    if (address)
    {
        bool ipv6 = (address.GetAddressType() == NetworkAddressType::IPv6);

        if (ipv6) o << "[";

        o << address.GetAddress();

        if (ipv6) o << "]";

        auto port = address.GetPort();

        if (port > 0) o << ":" << port;
    }

    return o;
}

/*
 *  operator<<() for NetworkAddressType
 *
 *  Description:
 *      Streaming operator to produce a human-friendly version of the
 *      NetworkAddressType value.
 *
 *  Parameters:
 *      o [out]
 *          Output stream.
 *
 *      type [in]
 *          The NetworkAddressType to transform into readable text.
 *
 *  Returns:
 *      The output stream reference.
 *
 *  Comments:
 *      None.
 */
std::ostream &operator<<(std::ostream &o, const NetworkAddressType &type)
{
    o << NetworkAddress::GetAddressTypeString(type);

    return o;
}

} // namespace Terra::NetUtil
