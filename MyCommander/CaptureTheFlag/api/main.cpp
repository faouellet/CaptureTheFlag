#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "Handshaking.h"
#include "CommanderFactory.h"
#include "NetworkCommanderClient.h"
#include "boost/thread/detail/thread.hpp"

using namespace std;

using namespace boost::asio;
using namespace boost::asio::ip;

int main(int argc, char** argv)
{
    string HOST = "localhost";
    string PORT = "41041";
    string commanderName;

    CommanderFactory& commanderFactory = CommanderFactory::getInstance();

    if ((argc == 1) && (commanderFactory.getCommanderCount() == 1))
    {
        commanderName = commanderFactory.getCommanderNames()[0];
    }
    else if (argc == 2)
    {
        commanderName = argv[1];
    }
    else if ((argc == 3) && (commanderFactory.getCommanderCount() == 1))
    {
        HOST = argv[1];
        PORT = argv[2];
        commanderName = commanderFactory.getCommanderNames()[0];
    }
    else if (argc == 4)
    {
        HOST = argv[1];
        PORT = argv[2];
        commanderName = argv[3];
    }
    else
    {
        cerr << "Usage: client [<hostname> <port>] [<commander_name>]" << endl;
        cerr << "eg. client localhost 41041 MyCommander" << endl;
        return 1;
    }

    if (!CommanderFactory::getInstance().exists(commanderName))
    {
        cerr << "Unable to create commander '" << commanderName << "'." << endl;
        return 1;
    }

    io_service io_service;
    tcp::socket socket(io_service);

    cout << "Connecting to " << HOST << ":" << PORT << endl;
    bool connected = false;
    for (int i = 0; i < 1000; ++i)
    {
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(HOST, PORT);
        tcp::resolver::iterator iterator = resolver.resolve(query);
        boost::system::error_code ec;
        connect(socket, iterator, ec);
        if (!ec)
        {
            cout << "Connected!" << endl;
            connected = true;
            break;
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }

    if (!connected)
    {
        cerr << "Unable to connect to " << HOST << ":" << PORT << endl;
        return 1;
    }

    NetworkCommanderClient wrapper(socket, commanderName);
    wrapper.run();

    return 0;
}
