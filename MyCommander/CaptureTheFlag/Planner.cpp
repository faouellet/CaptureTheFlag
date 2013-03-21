#include "Planner.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>

const int Planner::M_NBSTATES = 64;
const std::string Planner::M_PLANFILE = "Plan.txt";

void Planner::Init(const std::unique_ptr<GameInfo> & in_Game, const bool in_LoadFromDisk, const float in_LearningRate, const float in_Epsilon)
{
	m_LearningRate =in_LearningRate;
	m_Epsilon = in_Epsilon;
	InitRewards();

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

// TODO : Those are local rewards, should do something about global rewards
void Planner::InitRewards()
{
	for(int i = 0; i < M_NBSTATES; ++i)
	{
		m_Rewards[std::make_pair<Actions, State>(GetEnemyFlag, i)] = 0;
	}
	m_Rewards[std::make_pair<Actions, State>(GetEnemyFlag,       AgentCarryFlag)]    = 240;

	for(int i = 0; i < M_NBSTATES; ++i)
	{
		m_Rewards[std::make_pair<Actions, State>(WaitEnemyBase, i)] = -32;
	}
	m_Rewards[std::make_pair<Actions, State>(WaitEnemyBase,      AgentCarryFlag)]    = 80;
	m_Rewards[std::make_pair<Actions, State>(WaitEnemyBase,      TeammateCarryFlag)] = 120;

	for(int i = 0; i < M_NBSTATES; ++i)
	{
		m_Rewards[std::make_pair<Actions, State>(KillFlagCarrier, i)] = -80;
	}
	m_Rewards[std::make_pair<Actions, State>(KillFlagCarrier,    AgentCarryFlag)]    = 80;
	m_Rewards[std::make_pair<Actions, State>(KillFlagCarrier,    TeammateCarryFlag)] = 40;

	for(int i = 0; i < M_NBSTATES; ++i)
	{
		m_Rewards[std::make_pair<Actions, State>(Defend, i)] = 10;
	}
	m_Rewards[std::make_pair<Actions, State>(Defend,             AgentCarryFlag)]    = -80;

	for(int i = 0; i < M_NBSTATES; ++i)
	{
		m_Rewards[std::make_pair<Actions, State>(SupportFlagCarrier, i)] = -80;
	}
	m_Rewards[std::make_pair<Actions, State>(SupportFlagCarrier, AgentCarryFlag)]    = 240;

	for(int i = 0; i < M_NBSTATES; ++i)
	{
		m_Rewards[std::make_pair<Actions, State>(ReturnToBase, i)] = -120;
	}
	m_Rewards[std::make_pair<Actions, State>(ReturnToBase,       AgentCarryFlag)]    = 320;
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
	double l_BestValue = -300.f;
	double l_Value;

	for(int i = FirstAction; i <= LastAction; ++i)
	{
		l_Value = m_QValues[std::make_pair<Actions, State>(static_cast<Actions>(i), in_State)];
		if(l_Value > l_BestValue)
			l_BestAction = static_cast<Actions>(i);
	}

	return l_BestAction;
}
