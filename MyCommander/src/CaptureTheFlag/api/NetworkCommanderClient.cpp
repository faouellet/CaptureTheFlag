#include "NetworkCommanderClient.h"

#include <cassert>

#ifdef _WIN32
#pragma warning(push, 3) // prevent warnings from json headers
#endif
#include <json_spirit_reader_template.h>
#include <json_spirit_writer_template.h>
#ifdef _WIN32
#pragma warning(pop)
#endif

#ifdef _LOG_PERF
#include <fstream>
#include <iostream>
#endif

#include <boost/thread.hpp>

#include "CommanderFactory.h"
#include "json.h"
#include "Vector2.h"


NetworkCommanderClient::NetworkCommanderClient(tcp::socket& socket, const std::string& commanderName)
    :   m_socket(socket)
    ,   m_commander(CommanderFactory::getInstance().create(commanderName))
    ,   m_bufferMem()
{
    m_bufferMem.reserve(1024 * 1024);
}


NetworkCommanderClient::~NetworkCommanderClient() { }

void NetworkCommanderClient::performHandshaking()
    {
        std::string message = readLine();
        if (message != "<connect>")
        {
            cerr << "Expected connect message from the game server. Received: " << message << endl;
            assert(false);
            exit(1);
        }

        json_spirit::mValue value;
        std::string connectServerJson = readLine();
        json_spirit::read_string(connectServerJson, value);
        auto connectServer = fromJSON<ConnectServer>(value);
        if (!connectServer->validate())
        {
            assert(false);
            exit(1);
        }

        ConnectClient connectClient;
        connectClient.m_commanderName = m_commander->getName();
        connectClient.m_language = "C++";
        std::string connectClientJson = toJSON(connectClient);
        std::string connectMessage = "<connect>\n" + connectClientJson + "\n";
        boost::system::error_code ec;
        boost::asio::write(m_socket, buffer(connectMessage), ec);
        if (ec)
        {
            cerr << "Error sending data to server: " << ec.message() << endl;
            assert(false);
            exit(1);
        }
    }

void NetworkCommanderClient::run()
{
    performHandshaking();

    bool initialized = false;
    bool tickRequired = false;

#ifdef _LOG_PERF
	boost::chrono::high_resolution_clock::time_point l_Start;
#endif

    for (;;)
    {
        bool moreData = true;
        while (moreData)
        {
            std::string message = readLineNonBlocking();
            if (message == "")
            {
                moreData = false;
            }
            else if (message == "<initialize>")
            {
                assert(!initialized);
                std::string levelInfoJson = readLine();
                std::string gameInfoJson = readLine();
                initializeCommanderGameData(levelInfoJson, gameInfoJson);
                m_commander->initialize();
                sendReady();
                initialized = true;
            }
            else if (message == "<tick>")
            {
                assert(initialized);
                std::string gameInfoJson = readLine();
                updateCommanderGameData(gameInfoJson);
                tickRequired = true;
				//break;
            }
            else if (message == "<shutdown>")
            {
                assert(initialized);
                m_commander->shutdown();
#ifdef _LOG_PERF
				std::ofstream l_FileStream("Perf.txt", std::ios::out | std::ios::binary);
				if(l_FileStream.is_open())
				{
					for(unsigned i = 0; i < m_Durations.size(); ++i)
					{
						l_FileStream << "Tick:" << i << " Duration: " << m_Durations[i].count() << std::endl;
					}
				}
#endif
                return;
            }
            else
            {
                cerr <<  "Received unexpected message from server: " << message << endl;
                assert(false);
                exit(1);
            }
        }

        if (tickRequired)
        {
#ifdef _LOG_PERF
			l_Start = boost::chrono::high_resolution_clock::now();
#endif
            m_commander->tick();
            sendCommands();
            tickRequired = false;
#ifdef _LOG_PERF
			m_Durations.push_back(boost::chrono::high_resolution_clock::now() - l_Start);
#endif
        }
          
        boost::this_thread::sleep(boost::posix_time::microseconds(100));
    }
}

void NetworkCommanderClient::readAllDataFromSocket()
{
    const size_t maxTempDataSize = 16 * 1024;
    unsigned char tempData[maxTempDataSize];
    m_socket.non_blocking(true);
    for (;;)
    {
        boost::system::error_code ec;
        size_t numBytes = read(m_socket, buffer(tempData, maxTempDataSize), ec);
        if (numBytes > 0)
        {
            m_bufferMem.insert(m_bufferMem.end(), tempData, tempData + numBytes);
        }
        if ((ec == error::would_block) || (ec == error::try_again) || (ec == error::eof))
        {
            break;
        }
        else if (ec)
        {
            cerr << "Error receiving data from server: " << ec.message() << endl;
            assert(false);
            exit(1);
        }
    }
    m_socket.non_blocking(false);
}
    
std::string NetworkCommanderClient::readLineNonBlocking()
{
    readAllDataFromSocket();
    vector<unsigned char>::iterator newlineIndex = find(m_bufferMem.begin(), m_bufferMem.end(), '\n');
    if (newlineIndex == m_bufferMem.end())
    {
        return "";
    }
    std::string line(m_bufferMem.begin(), newlineIndex);
    newlineIndex++;
    m_bufferMem.erase(m_bufferMem.begin(), newlineIndex);     
    return line;
}

std::string NetworkCommanderClient::readLine()
{
    std::string line;
    do 
    {
        line = readLineNonBlocking();
    } while (line == "");
    return line;
}

void NetworkCommanderClient::initializeCommanderGameData(const std::string&  levelInfoJson, const std::string&  gameInfoJson)
{
    json_spirit::mValue value;
	// NOTE : Why was this needed ?
	 boost::this_thread::sleep(boost::posix_time::seconds(20));
    json_spirit::read_string(levelInfoJson, value);
    m_commander->m_level = fromJSON<LevelInfo>(value);

    json_spirit::read_string(gameInfoJson, value);
    m_commander->m_game = fromJSON<GameInfo>(value);
    fixupGameInfoReferences(*m_commander->m_game);
}

void NetworkCommanderClient::updateCommanderGameData(const std::string& gameInfoJson)
{
    json_spirit::mValue value;
    json_spirit::read_string(gameInfoJson, value);
    unique_ptr<GameInfo> game = fromJSON<GameInfo>(value);
    mergeGameInfo(*m_commander->m_game, *game);

    m_commander->m_game->bots_alive.clear();
    m_commander->m_game->bots_available.clear();
    m_commander->m_game->enemyFlags.clear();

    for(map<string, unique_ptr<FlagInfo> >::iterator i=m_commander->m_game->flags.begin(), end=m_commander->m_game->flags.end(); i!=end; ++i)
    {
        FlagInfo* flag = i->second.get();
        if(flag->team == m_commander->m_game->team)
            continue;

        m_commander->m_game->enemyFlags.push_back(flag);
    }

    for(vector<BotInfo*>::iterator i=m_commander->m_game->team->members.begin(), end=m_commander->m_game->team->members.end(); i!=end; ++i)
    {
        BotInfo* bot = *i;

        if(!bot || bot->health <= 0.0f)
            continue;

        m_commander->m_game->bots_alive.push_back(bot);

        if(*bot->state == BotInfo::STATE_IDLE)
            m_commander->m_game->bots_available.push_back(bot);
    }
}

void NetworkCommanderClient::sendReady()
{
    std::string message = "<ready>\n";
    boost::system::error_code ec;
    boost::asio::write(m_socket, buffer(message), ec);
    if (ec)
    {
        cerr << "Error sending <ready> data to server: " << ec.message() << endl;
        assert(false);
        exit(1);
    }
}

void NetworkCommanderClient::sendCommands()
{
    for (auto command = m_commander->m_commands.begin(); command != m_commander->m_commands.end(); ++command)
    {
        std::string commandJson = toJSON(**command);
        std::string message = "<command>\n" + commandJson + "\n";
        boost::system::error_code ec;
        boost::asio::write(m_socket, buffer(message), ec);
        if (ec)
        {
            cerr << "Error sending <command> data to server: " << ec.message() << endl;
            assert(false);
            exit(1);
        }
    }
    m_commander->m_commands.clear();
}
