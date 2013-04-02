#ifndef PLANNING_FIXTURE_H
#define PLANNING_FIXTURE_H

#include "Planner.h"
#include "Planner.cpp"

#include <memory>

#include "api\GameInfo.h"
#include "api\json.h"

#include "Utils.h"

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* PlanningFixture
*/
struct PlanningFixture
{
	static const double MAX_DECISION_TIME;

	static const Planner::State M_INITSTATE;
	static const Planner::State M_SCORESTATE;
	static const Planner::State M_SUPPORTSTATE;
	
	Planner m_Plan;

	std::unique_ptr<GameInfo> m_GameInitInfo;
	std::unique_ptr<GameInfo> m_GameTickInfo;

	PlanningFixture()
	{
		std::string l_GameInitStr = ReadAllFile("GameInit.json");
		std::string l_GameTickStr = ReadAllFile("SmallGameTick.json");

		json_spirit::mValue l_GameInitValue;
		json_spirit::mValue l_GameTickValue;
		
		json_spirit::read_string(l_GameInitStr, l_GameInitValue);
		json_spirit::read_string(l_GameTickStr, l_GameTickValue);

		m_GameInitInfo = fromJSON<GameInfo>(l_GameInitValue);
		m_GameTickInfo = fromJSON<GameInfo>(l_GameTickValue);
	}

	bool TestPerformance()
	{
		std::vector<boost::chrono::duration<double, boost::milli>> l_Durations(100);
		boost::chrono::high_resolution_clock::time_point l_Start;
		
		for(int i = 0; i < 100; ++i)
		{
			l_Start = boost::chrono::high_resolution_clock::now();
//			m_Plan.GetNextAction(*(m_GameTickInfo->team->members.begin()), M_SUPPORTSTATE);
			l_Durations[i] = boost::chrono::high_resolution_clock::now() - l_Start;
		}

		std::vector<double> l_Times = ToVectorOfDouble(l_Durations);
		return ComputeMean(l_Times) < MAX_DECISION_TIME;
	}

	Planner::State GetBotState(const std::unique_ptr<GameInfo> & in_GameInfo, const std::unique_ptr<BotInfo> & in_Bot)
	{
		Planner::State l_State = 0;

		if(in_GameInfo->flags["BlueFlag"]->position == in_GameInfo->teams["Blue"]->flagSpawnLocation)
		{
			l_State |= Planner::AgentFlagAtBase;
		}
		if(!in_GameInfo->flags["BlueFlag"]->carrier && 
			in_GameInfo->flags["BlueFlag"]->position != in_GameInfo->teams["Blue"]->flagSpawnLocation)
		{
			l_State |= Planner::AgentFlagDropped;
		}
		if(in_GameInfo->flags["RedFlag"]->position == in_GameInfo->teams["Red"]->flagSpawnLocation)
		{
			l_State |= Planner::OpponentFlagAtBase;
		}
		if(!in_GameInfo->flags["RedFlag"]->carrier && 
			in_GameInfo->flags["RedFlag"]->position != in_GameInfo->teams["Red"]->flagSpawnLocation)
		{
			l_State |= Planner::OpponentFlagDropped;
		}
		if(in_Bot->flag)
		{
			l_State |= Planner::AgentCarryFlag;
		}
		else if(std::any_of(in_GameInfo->bots_alive.begin(), in_GameInfo->bots_alive.end(), 
			[](const BotInfo* in_Bot){ return in_Bot->flag; }))
		{
			l_State |= Planner::TeammateCarryFlag;
		}

		return l_State;
	}
};

const double PlanningFixture::MAX_DECISION_TIME = 80.0;

const Planner::State PlanningFixture::M_INITSTATE = 5;
const Planner::State PlanningFixture::M_SCORESTATE = 17;
const Planner::State PlanningFixture::M_SUPPORTSTATE = 33;

#endif // PLANNING_FIXTURE_H
