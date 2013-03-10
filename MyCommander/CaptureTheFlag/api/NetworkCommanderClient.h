#ifndef NETWORK_COMMANDER_CLIENT_H
#define NETWORK_COMMANDER_CLIENT_H

#include <boost/asio.hpp>
#ifdef _LOG_PERF
#include <boost/chrono/chrono.hpp>
#endif

#include "Commander.h"

using namespace boost::asio;
using namespace boost::asio::ip;

class NetworkCommanderClient
{
public:
    NetworkCommanderClient(tcp::socket& socket, const std::string& commanderName);
    ~NetworkCommanderClient();
    void performHandshaking();
    void run();

private:
    void readAllDataFromSocket();
    std::string readLineNonBlocking();
    std::string readLine();
    void initializeCommanderGameData(const std::string&  levelInfoJson, const std::string&  gameInfoJson);
    void updateCommanderGameData(const std::string& gameInfoJson);
    void sendReady();
    void sendCommands();

private:
    tcp::socket& m_socket;
    std::unique_ptr<Commander> m_commander;
    std::vector<unsigned char> m_bufferMem;

#ifdef _LOG_PERF
	std::vector<boost::chrono::duration<double, boost::milli>> m_Durations;
#endif

};

#endif // NETWORK_COMMANDER_CLIENT_H
