/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*/

#include "Planner.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>

const int Planner::M_NBSTATES = 64;
const std::string Planner::M_PLANFILE = "Plan.txt";

void Planner::Init(const std::unique_ptr<GameInfo> & in_Game, const bool in_LoadFromDisk, const double in_LearningRate, 
				   const double in_Epsilon, const double in_Discount)
{
	m_LearningRate =in_LearningRate;
	m_Epsilon = in_Epsilon;
	m_Discount = in_Discount;

	if(in_LoadFromDisk)
	{
		LoadPlanFromDisk(M_PLANFILE);
	}

	std::for_each(in_Game->team->members.begin(), in_Game->team->members.end(), [this](std::vector<BotInfo*>::value_type in_Bot)
	{
		m_LastOrders[in_Bot->name] = std::make_pair<Actions, State>(FirstAction, 0);
	});
}

bool Planner::LoadPlanFromDisk(const std::string & in_Filename)
{
	std::ifstream l_FileStream(in_Filename);
	std::string l_Content;
	if(l_FileStream.is_open())
	{
		l_FileStream.seekg(0, std::ios::end);
		unsigned l_Size = static_cast<unsigned>(l_FileStream.tellg());
		l_Content.resize(l_Size);
		l_FileStream.seekg(0, std::ios::beg);
		l_FileStream.read(&l_Content[0], l_Content.size());
		l_FileStream.close();
	}
	return QValuesFromString(l_Content);
}

bool Planner::WritePlanToDisk(const std::string & in_Filename)
{
	bool l_Ok = false;
	std::ofstream l_FileStream(in_Filename);
	if(l_FileStream.is_open())
	{
		std::string l_QValuesStr = QValuesToString();
		if(!l_QValuesStr.empty())
		{
			l_FileStream << "{" << std::endl;
			l_FileStream << l_QValuesStr << std::endl;
			l_FileStream << "}";
			l_Ok = true;
		}
		l_FileStream.close();
	}
	return l_Ok;
}

Planner::Actions Planner::GetNextAction(const BotInfo* in_Bot, const Planner::State & in_CurrentState, const int in_TeamScore,
										const int in_EnemyScore, const std::vector<MatchCombatEvent> & in_Events)
{
#ifdef _TRAIN
	ActionState l_PreviousActionState = m_LastOrders[in_Bot->name];
	double l_Reward = ComputeReward(in_Bot, in_CurrentState, in_TeamScore, in_EnemyScore, l_PreviousActionState.first, in_Events);
	Actions l_BestAction = GetBestAction(in_CurrentState);
	double l_Delta = l_Reward + m_Discount * m_QValues[std::make_pair(l_BestAction, in_CurrentState)] - m_QValues[l_PreviousActionState];
	m_QValues[l_PreviousActionState] += m_LearningRate * l_Delta;

	Actions l_Action;

	if(m_UniformRealDistribution(m_RandomEngine) < m_Epsilon)
	{
		l_Action = static_cast<Actions>(m_RandomGenerator() % 8);
	}
	else
	{
		l_Action = GetBestAction(in_CurrentState);
	}
	m_LastOrders[in_Bot->name] = std::make_pair(l_Action, in_CurrentState);
	return l_Action == l_PreviousActionState.first ? None : l_Action;
#else
	return GetBestAction(in_CurrentState);
#endif
}

// TODO : Those are local rewards, should do something about global rewards
int Planner::ComputeReward(const BotInfo * in_Bot, const Planner::State & in_CurrentState, const int in_TeamScore, const int in_EnemyScore,
				  const Planner::Actions in_CurrentAction, const std::vector<MatchCombatEvent> & in_Events)
{
	int l_Reward = 0;
	auto l_KilledIt = std::find_if(in_Events.begin(), in_Events.end(), [&in_Bot](const MatchCombatEvent & in_Event)
			{
				return in_Event.type == MatchCombatEvent::TYPE_KILLED && in_Event.killedEventData.subject == in_Bot;
			});
	if(l_KilledIt != in_Events.end())
		return -300;

	auto l_EventIt = std::find_if(in_Events.begin(), in_Events.end(), [](const MatchCombatEvent & in_Event)
			{
				return in_Event.type == MatchCombatEvent::TYPE_FLAG_CAPTURED || in_Event.type == MatchCombatEvent::TYPE_FLAG_PICKEDUP;
			});

	// TODO : When scoring instead of capturing ?
	if(m_TeamScore < in_TeamScore)
	{
		m_TeamScore = in_TeamScore;
		l_Reward += 1600;
	}
	else
	{
		m_EnemyScore = in_EnemyScore;
		l_Reward -= 400;
	}

	switch(in_CurrentAction)
	{
		case GetEnemyFlag:
		{
			if(l_EventIt != in_Events.end()
				&& ((l_EventIt->type == MatchCombatEvent::TYPE_FLAG_CAPTURED && l_EventIt->flagCapturedEventData.instigator == in_Bot)
				|| (l_EventIt->type == MatchCombatEvent::TYPE_FLAG_PICKEDUP && l_EventIt->flagPickupEventData.instigator == in_Bot)))
				l_Reward += 240;
		}
		case WaitEnemyBase:
		{
			if(l_EventIt != in_Events.end())
			{
				if(l_EventIt->type == MatchCombatEvent::TYPE_FLAG_CAPTURED && l_EventIt->flagCapturedEventData.instigator == in_Bot
					|| l_EventIt->type == MatchCombatEvent::TYPE_FLAG_PICKEDUP && l_EventIt->flagPickupEventData.instigator == in_Bot)
					l_Reward -= 80;
				else if(l_EventIt->type == MatchCombatEvent::TYPE_FLAG_CAPTURED && l_EventIt->flagCapturedEventData.instigator->team == in_Bot->team
					|| l_EventIt->type == MatchCombatEvent::TYPE_FLAG_PICKEDUP && l_EventIt->flagPickupEventData.instigator->team == in_Bot->team)
					l_Reward += 120;
				else
					l_Reward -= 32;
			}
			
		}
		case KillFlagCarrier:
		{
			// TODO : Should revise
			if(l_EventIt != in_Events.end())
			{
				if(l_EventIt->type == MatchCombatEvent::TYPE_FLAG_CAPTURED && l_EventIt->flagCapturedEventData.instigator == in_Bot
					|| l_EventIt->type == MatchCombatEvent::TYPE_FLAG_PICKEDUP && l_EventIt->flagPickupEventData.instigator == in_Bot)
					l_Reward += 80;
				else if(l_EventIt->type == MatchCombatEvent::TYPE_FLAG_CAPTURED && l_EventIt->flagCapturedEventData.instigator->team == in_Bot->team
					|| l_EventIt->type == MatchCombatEvent::TYPE_FLAG_PICKEDUP && l_EventIt->flagPickupEventData.instigator->team == in_Bot->team)
					l_Reward += 40;
				else
					l_Reward -= 80;
			}
		}
		case Defend:
		{
			if(l_EventIt != in_Events.end())
			{
				if(l_EventIt->type == MatchCombatEvent::TYPE_FLAG_CAPTURED && l_EventIt->flagCapturedEventData.instigator == in_Bot
					|| l_EventIt->type == MatchCombatEvent::TYPE_FLAG_PICKEDUP && l_EventIt->flagPickupEventData.instigator == in_Bot
					|| in_Bot->flag)
					l_Reward -= 80;
				if(l_EventIt->type == MatchCombatEvent::TYPE_FLAG_CAPTURED && l_EventIt->flagCapturedEventData.instigator->team != in_Bot->team
					|| l_EventIt->type == MatchCombatEvent::TYPE_FLAG_PICKEDUP && l_EventIt->flagPickupEventData.instigator->team != in_Bot->team)
					l_Reward += 160;
			}
			else if(in_CurrentState & AgentCarryFlag)
				l_Reward -= 80;
			else
				l_Reward += 10;
		}
		case SupportFlagCarrier:
		{
			if(l_EventIt != in_Events.end())
			{
				if(l_EventIt->type == MatchCombatEvent::TYPE_FLAG_CAPTURED && l_EventIt->flagCapturedEventData.subject != in_Bot->team->flag
					|| l_EventIt->type == MatchCombatEvent::TYPE_FLAG_PICKEDUP && l_EventIt->flagPickupEventData.subject != in_Bot->team->flag)
					l_Reward += 80;
			}
			else if(in_CurrentState & TeammateCarryFlag)
				l_Reward += 8;
			else
				l_Reward -= 8;
		}
		case ReturnToBase:
		{
			if(m_TeamScore < in_TeamScore)
			{
				m_TeamScore = in_TeamScore;
				l_Reward += 320;
			}
			else if(in_Bot->flag || in_CurrentState & AgentCarryFlag)
				l_Reward += 80;
			else
				l_Reward -= 120;
		}
	}

	return l_Reward;
}

std::string Planner::QValuesToString() const
{
	std::stringstream l_QValuesStream;
	l_QValuesStream << "\t\"QValues\": [" << std::endl;

	for(auto l_It1 = m_QValues.begin(); l_It1 != m_QValues.end(); ++l_It1)
	{
		l_QValuesStream << "\t\t{ \"State\":" << l_It1->first.second << ", ";
		l_QValuesStream << "\"Action\":" << l_It1->first.first << ", ";
		l_QValuesStream << "\"Value\":" << l_It1->second << " }," << std::endl;
	}

	return l_QValuesStream.str().erase(l_QValuesStream.str().length()-2) + "\n\t]";
}

bool Planner::QValuesFromString(const std::string in_Content)
{
	std::regex r("\"State\":([0-9]+), \"Action\":([0-9]+), \"Value\":([0-9]+\\.[0-9]+)");
	for(std::sregex_iterator l_It(in_Content.begin(), in_Content.end(), r); 
		l_It != std::sregex_iterator(); ++l_It)
	{
		m_QValues[std::make_pair(static_cast<Actions>(atoi((*l_It)[2].str().c_str())),
			atoi((*l_It)[1].str().c_str()))] = atof((*l_It)[3].str().c_str());
	}

	return true;
}

Planner::Actions Planner::GetBestAction(const State in_State)
{
	Actions l_BestAction = FirstAction;
	double l_BestValue = std::numeric_limits<double>::infinity();
	double l_Value;

	for(int i = FirstAction; i <= LastAction; ++i)
	{
		l_Value = m_QValues[std::make_pair<Actions, State>(static_cast<Actions>(i), in_State)];
		if(l_Value > l_BestValue)
		{
			l_BestAction = static_cast<Actions>(i);
			l_BestValue = l_Value;
		}
	}

	return l_BestAction;
}
