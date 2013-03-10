#include "Planner.h"

Planner::Planner(const float in_LearningRate, const float in_Epsilon) :
	m_LearningRate(in_LearningRate), m_Epsilon(in_Epsilon) { }

bool Planner::LoadPlanFromDisk(const std::string & in_Filename)
{
	return true;
}

bool Planner::WritePlanToDisk(const std::string & in_Filename)
{
	return true;
}

Planner::Action Planner::GetNextAction(const Planner::Action & in_CurrentAction, 
							  const Planner::State & in_CurrentState, const Planner::State & in_PreviousState)
{
	return Planner::GetEnemyFlag;
}

void Planner::InitRewards()
{

}
