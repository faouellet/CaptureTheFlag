#include "Planner.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>

Planner::Planner(const std::unique_ptr<GameInfo> & in_Game, const float in_LearningRate, const float in_Epsilon) :
	m_LearningRate(in_LearningRate), m_Epsilon(in_Epsilon) 
{
	InitRewards();
	std::for_each(in_Game->team->members.begin(), in_Game->team->members.end(), [this](std::vector<BotInfo*>::value_type in_Bot)
	{
		m_LastOrders[in_Bot->name] = std::make_pair<Actions, State>(FirstAction, 0);
	});
}

bool Planner::LoadPlanFromDisk(const std::string & in_Filename)
{
	bool l_Ok = false;
	std::ifstream l_FileStream(in_Filename);
	std::string l_Content;
	if(l_FileStream.is_open())
	{
		// First line, don't care
		l_FileStream >> l_Content;

		l_FileStream >> l_Content;
		
		if(QValuesFromString(l_Content))
		{
			l_Ok = true;
		}

		l_FileStream.close();
	}
	return l_Ok;
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
			l_FileStream << "}" << std::endl;
			l_Ok = true;
		}
		l_FileStream.close();
	}
	return l_Ok;
}

Planner::Actions Planner::GetNextAction(const std::string & in_BotID, const Planner::State & in_CurrentState)
{
#ifdef _TRAIN
	ActionState l_PreviousActionState = m_LastOrders[in_BotID];
	float l_Reward = m_Rewards[l_PreviousActionState];
	Action l_BestAction = GetBestAction(in_CurrentState);
	float l_Delta = m_Epsilon * l_BestAction - m_QValues[l_PreviousActionState];
	m_QValues[l_PreviousActionState] += m_LearningRate * l_Delta;

	if(m_RandomGenerator() < m_Epsilon)
	{
		return static_cast<Action>(m_RandomGenerator() % 8);
	}
	else
	{
		return l_BestAction;
	}
#else
	return GetBestAction(in_CurrentState);
#endif
}

// TODO : To revise
void Planner::InitRewards()
{
	m_Rewards[std::make_pair<Actions, States>(GetEnemyFlag,       AgentCarryFlag)]    = 240;
	m_Rewards[std::make_pair<Actions, States>(WaitEnemyBase,      AgentCarryFlag)]    = 80;
	m_Rewards[std::make_pair<Actions, States>(WaitEnemyBase,      TeammateCarryFlag)] = 120;
	m_Rewards[std::make_pair<Actions, States>(KillFlagCarrier,    AgentCarryFlag)]    = 80;
	m_Rewards[std::make_pair<Actions, States>(KillFlagCarrier,    TeammateCarryFlag)] = 40;
	m_Rewards[std::make_pair<Actions, States>(Defend,             AgentCarryFlag)]    = -80;
	m_Rewards[std::make_pair<Actions, States>(SupportFlagCarrier, AgentCarryFlag)]    = 240;
	m_Rewards[std::make_pair<Actions, States>(GetEnemyFlag,       AgentCarryFlag)]    = 240;
	m_Rewards[std::make_pair<Actions, States>(GetEnemyFlag,       AgentCarryFlag)]    = 240;
}

std::string Planner::QValuesToString() const
{
	std::stringstream l_QValuesStream;
	l_QValuesStream << "\"QValues\": [";

	for(auto l_It1 = m_QValues.begin(); l_It1 != m_QValues.end(); ++l_It1)
	{
		l_QValuesStream << "{ \"State\":" << l_It1->first.first << ", ";
		l_QValuesStream << "\"Action\":" << l_It1->first.second << ", ";
		l_QValuesStream << "\"Value\":" << l_It1->second << "}," << std::endl;
	}

	l_QValuesStream << "]";

	return l_QValuesStream.str();
}

bool Planner::QValuesFromString(const std::string in_Content)
{
	std::regex r("\"State\":([0-9]+), \"Action\":([0-9]+), \"Value\":([0-9]+)");
	for(std::sregex_token_iterator l_It(in_Content.begin(), in_Content.end, r); 
		l_It != std::sregex_token_iterator(); ++l_It)
	{
		l_It;
		// m_QValues[std::make_pair<Actions, States>()] = ;
	}

	return true;
}

Planner::Actions Planner::GetBestAction(const State in_State)
{
	Actions l_BestAction = FirstAction;
	float l_BestValue = -300.f;
	float l_Value;

	for(int i = FirstAction; i <= LastAction; ++i)
	{
		l_Value = m_QValues[std::make_pair<Actions, State>(static_cast<Actions>(i), in_State)];
		if(l_Value > l_BestValue)
			l_BestAction = static_cast<Actions>(i);
	}

	return l_BestAction;
}
