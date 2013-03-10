#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include "PlanningFixture.h"

#include <vector>

#include "Utils.h"

/*
* Planning Tests
* The goal of these tests is to make sure that the Planner can produce good
* strategies to match some given game states. They also make sure that its
* performance acceptable within the constraints of the game
*/
BOOST_FIXTURE_TEST_SUITE( PlanningTestSuite, PlanningFixture )

BOOST_AUTO_TEST_CASE( LoadWriteTest )
{
	std::string l_TestStr = "Test.json";

	Planner::State l_State1 = 5;
	Planner::State l_State2 = 0;
	Planner::State l_State3 = 31;
	
	std::map<Planner::State, Planner::Action> l_Plan;
	l_Plan[l_State1] = Planner::Defend;
	l_Plan[l_State2] = Planner::KillFlagCarrier;
	l_Plan[l_State3] = Planner::SupportFlagCarrier;

	BOOST_REQUIRE(m_Plan.WritePlanToDisk(l_TestStr));

	std::string l_PlanStr = ReadAllFile("Plan.json");
	std::string l_TestPlanStr = ReadAllFile(l_TestStr);
	BOOST_REQUIRE(l_PlanStr == l_TestPlanStr);

	BOOST_REQUIRE(m_Plan.LoadPlanFromDisk(l_TestStr));

	BOOST_REQUIRE(m_Plan.GetNextAction(Planner::Defend, l_State1, l_State1) == Planner::Defend);
	BOOST_REQUIRE(m_Plan.GetNextAction(Planner::KillFlagCarrier, l_State2, l_State2) == Planner::KillFlagCarrier);
	BOOST_REQUIRE(m_Plan.GetNextAction(Planner::SupportFlagCarrier, l_State3, l_State3) == Planner::SupportFlagCarrier);
}

BOOST_AUTO_TEST_CASE( QLearningCorrectnessTest )
{
	Planner::Action l_GoodAction = Planner::ReturnToBase;
	std::vector<Planner::Action> l_Actions(500);
	for(int i = 0; i < 500; ++i)
	{
		l_Actions[i] = m_Plan.GetNextAction(m_CurrentAction, m_CurrentState, m_PreviousState);
	}

	int l_Count = std::count_if(l_Actions.begin(), l_Actions.end(), [&l_GoodAction](const Planner::Action in_Action)
	{
		return in_Action == l_GoodAction;
	});
	float l_Ratio = static_cast<float>(l_Count) / l_Actions.size();

	BOOST_REQUIRE(0.09f < l_Ratio && l_Ratio < 0.11f);
	BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE( QLearningPerformanceTest )
{
	BOOST_REQUIRE(TestPerformance());
}

BOOST_AUTO_TEST_SUITE_END()
