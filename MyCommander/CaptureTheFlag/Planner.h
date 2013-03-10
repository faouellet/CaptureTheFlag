#ifndef PLANNER_H
#define PLANNER_H

#include <algorithm>
#include <map>
#include <random>
#include <set>
#include <string>
#include <utility>

/*
* Planner
* Makes the high level decisions for a group of bots.
* It uses Q-Learning to formulate the best plan possible given the game's state.
*/
class Planner
{
public:
	enum Action { GetEnemyFlag, WaitEnemyBase, KillFlagCarrier, Defend, SupportFlagCarrier, ReturnToBase };
	enum States { AgentFlagAtBase = 1, AgentFlagDropped = 2, OpponentFlagAtBase = 4, OpponentFlagDropped = 8, AgentCarryFlag = 16 };

	typedef size_t State;

private:
	typedef std::pair<Action, float> ActionValue;
	struct Comp
	{
		bool operator()(const ActionValue & in_ActVal1, const ActionValue & in_ActVal2)
		{
			return in_ActVal1.second > in_ActVal2.second;
		}
	};

private:
	float m_LearningRate;
	float m_Epsilon;
	std::map<State, Action> m_Plan;
	std::map<State, float> m_Rewards;
	std::map<State, std::set<ActionValue, Comp>> m_QValues;
	std::random_device m_RandomGenerator;

public:
	Planner(const float in_LearningRate = 0.1f, const float in_Epsilon = 0.1f);

	bool LoadPlanFromDisk(const std::string & in_Filename);
	bool WritePlanToDisk(const std::string & in_Filename);

	Action GetNextAction(const Action & in_CurrentAction, const State & in_CurrentState, const State & in_PreviousState);

private:
	void InitRewards();

};

#endif // PLANNER_H
