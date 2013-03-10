#ifndef HANDSHAKING_H
#define HANDSHAKING_H

#include <string>

struct ConnectServer
{
    ConnectServer()
    : m_expectedProtocolVersion("1.4")
    {
    }

    bool validate()
    {
        if (m_protocolVersion < m_expectedProtocolVersion)
        {
            fprintf(stderr, "This client version does not match network protocol version. Expected version %s received %s.\n", m_expectedProtocolVersion.c_str(), m_protocolVersion.c_str());
            fprintf(stderr, "Check the http://aisandbox.com/download page for an Capture The Flag SDK package.\n");
            return false;
        }
        else if (m_protocolVersion > m_expectedProtocolVersion)
        {
            fprintf(stderr, "This client version does not match network protocol version. Expected version %s received %s.\n", m_expectedProtocolVersion.c_str(), m_protocolVersion.c_str());
            fprintf(stderr, "Check the http://aisandbox.com/download page for an updated C++ starter kit.\n");
            return false;
        }

        return true;
    }

    std::string m_expectedProtocolVersion;
    std::string m_protocolVersion;
};

struct ConnectClient
{
    ConnectClient()
    {
    }

    std::string m_commanderName;
    std::string m_language;
};

#endif // HANDSHAKING_H
