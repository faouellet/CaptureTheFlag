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
	m_Cmd.setPrivates(InitGameInfo(m_GameInitValue), fromJSON<LevelInfo>(m_SmallLevelValue));

	BOOST_REQUIRE(TestInitPerformance());
}

BOOST_AUTO_TEST_CASE( MediumInitTest )
{
	m_Cmd.setPrivates(InitGameInfo(m_GameInitValue), fromJSON<LevelInfo>(m_MediumLevelValue));

	BOOST_REQUIRE(TestInitPerformance());
}

BOOST_AUTO_TEST_CASE( LargeInitTest )
{
	m_Cmd.setPrivates(InitGameInfo(m_GameInitValue), fromJSON<LevelInfo>(m_LargeLevelValue));

	BOOST_REQUIRE(TestInitPerformance());
}

BOOST_AUTO_TEST_CASE( SmallTickTest )
{
	m_Cmd.setPrivates(InitGameInfo(m_SmallGameTickValue), fromJSON<LevelInfo>(m_SmallLevelValue));

	BOOST_REQUIRE(TestTickPerformance());
}

BOOST_AUTO_TEST_CASE( MediumTickTest )
{
	m_Cmd.setPrivates(InitGameInfo(m_MediumGameTickValue), fromJSON<LevelInfo>(m_MediumLevelValue));

	BOOST_REQUIRE(TestTickPerformance());
}

BOOST_AUTO_TEST_CASE( LargeTickTest )
{
	m_Cmd.setPrivates(InitGameInfo(m_LargeGameTickValue), fromJSON<LevelInfo>(m_LargeLevelValue));

	BOOST_REQUIRE(TestTickPerformance());
}

BOOST_AUTO_TEST_SUITE_END()
