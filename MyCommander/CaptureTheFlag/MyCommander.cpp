#include "MyCommander.h"

#include <cassert>

#include "api/CommanderFactory.h"
#include "api/Commands.h"
#include "api/GameInfo.h"

REGISTER_COMMANDER(MyCommander);

std::string MyCommander::getName() const
{
    return "MyCommander";
}

void MyCommander::initialize()
{
    // TODO : Initialize the Navigator

	m_Planner.LoadPlanFromDisk("Plan.json");
}

void MyCommander::tick()
{
	for (size_t i=0; i< m_game->bots_available.size(); ++i)
    {
		auto l_Bot = m_game->bots_available[i];
		Planner::State l_CurrentState = GetBotState(l_Bot);
		Planner::Actions l_Action = m_Planner.GetNextAction(l_Bot->name, l_CurrentState);
		ActionToCommand(l_Action, l_Bot);
	}

    // TODO : Process continuous task with the time left (Ex: Cluster construction for HPA*)
}

void MyCommander::shutdown() 
{
	m_Planner.WritePlanToDisk("Plan.json");
}

Planner::State MyCommander::GetBotState(const BotInfo* in_Bot)
{
	Planner::State l_State = 0;

	if(m_game->team->flag->position == m_game->team->flagSpawnLocation)
	{
		l_State |= Planner::AgentFlagAtBase;
	}
	if(!m_game->team->flag->carrier && 
		m_game->team->flag->position != m_game->team->flagSpawnLocation)
	{
		l_State |= Planner::AgentFlagDropped;
	}
	if(m_game->enemyTeam->flag->position == m_game->enemyTeam->flagSpawnLocation)
	{
		l_State |= Planner::OpponentFlagAtBase;
	}
	if(!m_game->enemyTeam->flag->carrier && 
		m_game->enemyTeam->flag->position != m_game->enemyTeam->flagSpawnLocation)
	{
		l_State |= Planner::OpponentFlagDropped;
	}
	if(in_Bot->flag)
	{
		l_State |= Planner::AgentCarryFlag;
	}
	else if(std::any_of(m_game->bots_alive.begin(), m_game->bots_alive.end(), 
		[](const BotInfo* in_Bot){ return in_Bot->flag; }))
	{
		l_State |= Planner::TeammateCarryFlag;
	}

	return l_State;
}

void MyCommander::ActionToCommand(const Planner::Actions in_Action, const BotInfo* in_Bot)
{
	Command* l_Command;

	// TODO : Call the Navigator for the path
	switch (in_Action)
	{
		case Planner::GetEnemyFlag:
		{
			issue(new AttackCommand(in_Bot->name, m_game->enemyTeam->flagSpawnLocation, m_game->enemyTeam->flagSpawnLocation));
			break;
		}
		case Planner::WaitEnemyBase:
		{
			// If far -> attack toward enemy base
			if(sqrt(pow(m_game->enemyTeam->flagScoreLocation.x - in_Bot->position->x, 2) + 
				pow(m_game->enemyTeam->flagScoreLocation.y - in_Bot->position->y, 2)) > 0.f)
			{
				issue(new AttackCommand(in_Bot->name, m_game->enemyTeam->flagScoreLocation, m_game->enemyTeam->flagScoreLocation));
			}
			else // defend/patrol
			{
				// TODO : Should look around for incoming enemies
				issue(new DefendCommand(in_Bot->name));
			}
			break;
		}
		case Planner::KillFlagCarrier:
		{
			// TODO : Choose between trying to follow or trying to intercept the flag carrier
			break;
		}
		case Planner::Defend:
		{
			// TODO : Use other DefendCommand ??
			issue(new DefendCommand(in_Bot->name));
			break;
		}
		case Planner::SupportFlagCarrier:
		{
			issue(new AttackCommand(in_Bot->name, *(m_game->team->flag->carrier->position), m_game->team->flag->carrier->position));
			break;
		}
		case Planner::ReturnToBase:
		{
			issue(new ChargeCommand(in_Bot->name, m_game->team->flagScoreLocation));
			break;
		}
		default:
		{
			break;
		}
	}
}
