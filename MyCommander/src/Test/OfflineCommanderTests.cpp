#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include "OfflineFixture.h"

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* Offline Tests
* The goal of these tests is to make sure that the commander meet performance
* requirements in a static environment i.e. when he is not connected to the
* server running a game and there is no enemies to consider. These tests are 
* the bare minimum that the commander must pass to be considered acceptable in 
* terms of time performance.
*/

BOOST_FIXTURE_TEST_SUITE( OfflineCommanderTestSuite, OfflineFixture )

BOOST_AUTO_TEST_CASE( SmallInitTest )
{
	m_Cmd.setPrivates(InitGameInfo(m_GameInitValue, m_SmallLevel), m_SmallLevel);

	BOOST_REQUIRE(TestInitPerformance());
}

BOOST_AUTO_TEST_CASE( MediumInitTest )
{
	m_Cmd.setPrivates(InitGameInfo(m_GameInitValue, m_MediumLevel), m_MediumLevel);

	BOOST_REQUIRE(TestInitPerformance());
}

BOOST_AUTO_TEST_CASE( SmallTickTest )
{
	m_Cmd.setPrivates(InitGameInfo(m_SmallGameTickValue, m_SmallLevel), m_SmallLevel);
	m_Cmd.initialize();

	BOOST_REQUIRE(TestTickPerformance());
}

BOOST_AUTO_TEST_CASE( MediumTickTest )
{
	m_Cmd.setPrivates(InitGameInfo(m_MediumGameTickValue, m_MediumLevel), m_MediumLevel);
	m_Cmd.initialize();

	BOOST_REQUIRE(TestTickPerformance());
}

BOOST_AUTO_TEST_SUITE_END()
