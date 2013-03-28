#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include "PlanningFixture.h"

#include <vector>

#include "Utils.h"

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* Planning Tests
* The goal of these tests is to make sure that the Planner can produce good
* strategies to match some given game states. They also make sure that its
* performance acceptable within the constraints of the game
*/
BOOST_FIXTURE_TEST_SUITE( PlanningTestSuite, PlanningFixture )

BOOST_AUTO_TEST_CASE( LoadWriteTest )
{
	std::string l_TestStr = "Test.json";

	std::map<std::pair<Planner::Actions, Planner::State>, double> l_QValues;
	l_QValues[std::make_pair(Planner::SupportFlagCarrier, M_SUPPORTSTATE)] = 5.498; 
	l_QValues[std::make_pair(Planner::ReturnToBase, M_SCORESTATE)] = 3.14159;

	m_Plan.SetQValues(l_QValues);

	BOOST_REQUIRE(m_Plan.WritePlanToDisk(l_TestStr));

	std::string l_PlanStr = ReadAllFile("QValues.json");
	std::string l_TestPlanStr = ReadAllFile(l_TestStr);
	BOOST_REQUIRE(l_PlanStr == l_TestPlanStr);

	BOOST_REQUIRE(m_Plan.LoadPlanFromDisk(l_TestStr));

	m_Plan.Init(m_GameTickInfo, false, 0.1, -2.0);

	//BOOST_REQUIRE(m_Plan.GetNextAction("Blue0", M_SUPPORTSTATE) == Planner::SupportFlagCarrier);
	//BOOST_REQUIRE(m_Plan.GetNextAction("Blue1", M_SCORESTATE) == Planner::ReturnToBase);
}

// The correctness tests verify that the Q-Learning algorithm is correctly working.
// This means we have to go into _TRAIN mode.

BOOST_AUTO_TEST_CASE( QLearningInitCorrectnessTest )
{
	m_Plan.Init(m_GameInitInfo);
	Planner::Actions l_GoodAction = Planner::GetEnemyFlag;
	std::vector<Planner::Actions> l_Actions(500);

	Planner::State l_State = GetBotState(m_GameInitInfo, m_GameInitInfo->bots["Blue0"]);
	BOOST_REQUIRE(l_State == M_INITSTATE);

	for(int i = 0; i < 500; ++i)
	{
//		l_Actions[i] = m_Plan.GetNextAction(*(m_GameInitInfo->team->members.begin()), l_State);
	}

	int l_Count = std::count_if(l_Actions.begin(), l_Actions.end(), [&l_GoodAction](const Planner::Actions in_Action)
	{
		return in_Action == l_GoodAction;
	});
	float l_Ratio = static_cast<float>(l_Count) / l_Actions.size();

	// BOOST_REQUIRE(0.09f < l_Ratio && l_Ratio < 0.11f);
}

BOOST_AUTO_TEST_CASE( QLearningTickCorrectnessTest )
{
	m_Plan.Init(m_GameTickInfo);
	Planner::Actions l_GoodAction = Planner::ReturnToBase;
	std::vector<Planner::Actions> l_Actions(500);

	Planner::State l_State = GetBotState(m_GameTickInfo, m_GameTickInfo->bots["Blue1"]);
	BOOST_REQUIRE(l_State == M_SCORESTATE);

	for(int i = 0; i < 500; ++i)
	{
//		l_Actions[i] = m_Plan.GetNextAction(*(m_GameTickInfo->team->members.begin()), l_State);
	}

	int l_Count = std::count_if(l_Actions.begin(), l_Actions.end(), [&l_GoodAction](const Planner::Actions in_Action)
	{
		return in_Action == l_GoodAction;
	});
	float l_Ratio = static_cast<float>(l_Count) / l_Actions.size();

	// BOOST_REQUIRE(0.09f < l_Ratio && l_Ratio < 0.11f);
}

BOOST_AUTO_TEST_CASE( QLearningPerformanceTest )
{
	BOOST_REQUIRE(TestPerformance());
}

BOOST_AUTO_TEST_SUITE_END()
