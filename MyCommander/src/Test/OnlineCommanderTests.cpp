#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <stdlib.h>
#include <fstream>
#include <regex>

#include <boost/thread.hpp>

#include "OnlineFixture.h"
#include "api\NetworkCommanderClient.h"
#include "api\NetworkCommanderClient.cpp"

#include "Utils.h"

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* Online Tests
* The goal of these tests is to make sure that the commander meet performance
* requirements in a real game situation. This constitute the final test that 
* the commander must be pass to be considered successful
*/
BOOST_FIXTURE_TEST_SUITE( OnlineCommanderTestSuite, OnlineFixture )

BOOST_AUTO_TEST_CASE( OnlineTest )
{
	std::string HOST = "localhost";
    std::string PORT = "41041";
    std::string commanderName = "MyCommander";

    io_service io_service;
    tcp::socket socket(io_service);

	system("START /b Server.bat");

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
            connected = true;
            break;
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }

    NetworkCommanderClient wrapper(socket, commanderName);
    wrapper.run();

	std::string l_FileContent = ReadAllFile("Perf.txt");
	std::vector<std::string> l_Matches(std::sregex_token_iterator(l_FileContent.begin(), l_FileContent.end(), 
		std::regex("Duration: ([0-9]+\\.[0-9]+)"), 1), std::sregex_token_iterator());

	std::vector<double> l_Durations(l_Matches.size());
	std::transform(l_Matches.begin(), l_Matches.end(), l_Durations.begin(), 
		[](const std::string & in_Match){ return atof(in_Match.c_str()); });

	BOOST_REQUIRE(ComputeMean(l_Durations) < MAX_TIME_TICK);
}

BOOST_AUTO_TEST_SUITE_END()
