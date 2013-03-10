#ifndef PLANNING_FIXTURE_H
#define PLANNING_FIXTURE_H

#include "Planner.h"
#include "Planner.cpp"

#include "Utils.h"

struct PlanningFixture
{
	const static double MAX_DECISION_TIME;

	Planner m_Plan;
	Planner::State m_CurrentState;
	Planner::State m_PreviousState;
	Planner::Action m_CurrentAction;

	PlanningFixture()
	{
	}

	bool TestPerformance()
	{
		std::vector<boost::chrono::duration<double, boost::milli>> l_Durations(100);
		boost::chrono::high_resolution_clock::time_point l_Start;
		
		for(int i = 0; i < 100; ++i)
		{
			l_Start = boost::chrono::high_resolution_clock::now();
			m_Plan.GetNextAction(m_CurrentAction, m_CurrentState, m_PreviousState);
			l_Durations[i] = boost::chrono::high_resolution_clock::now() - l_Start;
		}

		std::vector<double> l_Times = ToVectorOfDouble(l_Durations);

		return ComputeMean(l_Times) < MAX_DECISION_TIME;
	}

};

// TODO : Find something more appropriate than a tick time
const double PlanningFixture::MAX_DECISION_TIME = 80.0;

#endif // PLANNING_FIXTURE_H
