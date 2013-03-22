#ifndef PLANNER_H
#define PLANNER_H

#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <utility>

#include "api\GameInfo.h"

/*
* Planner
* Makes the high level decisions for a group of bots.
* It uses Q-Learning to formulate the best plan possible given the game's state.
*/
class Planner
{
public:
	enum Actions { GetEnemyFlag = 1, WaitEnemyBase, KillFlagCarrier, Defend, SupportFlagCarrier, ReturnToBase,
		FirstAction = GetEnemyFlag, LastAction = ReturnToBase };

	enum States { BaseState = 0, AgentFlagAtBase = 1, AgentFlagDropped = 2, OpponentFlagAtBase = 4, OpponentFlagDropped = 8, 
		AgentCarryFlag = 16, TeammateCarryFlag = 32 };

	typedef size_t State;

private:
	typedef std::pair<Actions, State> ActionState;

private:
	double m_LearningRate;
	double m_Epsilon;
	double m_Discount;
	static const int M_NBSTATES;
	static const std::string M_PLANFILE;
	std::map<ActionState, double> m_Rewards;
	std::map<ActionState, double> m_QValues;
	std::default_random_engine m_RandomEngine;
	std::uniform_real_distribution<double> m_UniformRealDistribution;
	std::default_random_engine m_RandomGenerator;
	std::map<std::string, ActionState> m_LastOrders;

public:
	Planner() { m_UniformRealDistribution = std::uniform_real_distribution<double>(0.0, 1.0); }
	void Init(const std::unique_ptr<GameInfo> & in_Game, const bool in_LoadFromDisk = false, 
		const double in_LearningRate = 0.1, const double in_Epsilon = 0.4, const double in_Discount = 0.9995);

	bool LoadPlanFromDisk(const std::string & in_Filename);
	bool WritePlanToDisk(const std::string & in_Filename);

	Actions GetNextAction(const std::string & in_BotID, const State & in_CurrentState);

	void SetQValues(const std::map<ActionState, double> & in_QValues) { m_QValues = in_QValues; }

private:
	void InitRewards();
	std::string PlanToString() const;
	std::string QValuesToString() const;
	bool QValuesFromString(const std::string in_Content);
	Actions GetBestAction(const State in_State);

};

#endif // PLANNER_H
