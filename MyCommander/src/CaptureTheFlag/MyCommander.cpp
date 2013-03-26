/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*/

#include "MyCommander.h"

#include <cassert>
#include <iostream>

#include "api/CommanderFactory.h"
#include "api/Commands.h"
#include "api/GameInfo.h"

REGISTER_COMMANDER(MyCommander);

const std::string MyCommander::M_QVALUESFILE = "QValues.json";

std::string MyCommander::getName() const
{
    return "MyCommander";
}

void MyCommander::initialize()
{
	// m_Navigator.Init(m_level->blockHeights, m_level->height, m_level->width);
#ifdef _TRAIN
	m_Planner.Init(m_game, false);
#else
	m_Planner.Init(m_game, true);
#endif
}

void MyCommander::tick()
{
	if(!m_game->match->combatEvents.empty())
	{
		for (size_t i = 0; i< m_game->bots_available.size(); ++i)
		{
			auto l_Bot = m_game->bots_available[i];
			Planner::State l_CurrentState = GetBotState(l_Bot);
			Planner::Actions l_Action = m_Planner.GetNextAction(l_Bot, l_CurrentState, m_game->match->combatEvents);
			ActionToCommand(l_Action, l_Bot);
		}
	}

    // TODO : Process continuous task with the time left (Ex: Cluster construction for HPA*)
}

void MyCommander::shutdown() 
{
	m_Planner.WritePlanToDisk(M_QVALUESFILE);
}

Planner::State MyCommander::GetBotState(const BotInfo* in_Bot)
{
	Planner::State l_State = 0;

	if(m_game->team->flag->position == m_game->team->flagSpawnLocation
		&& !m_game->team->flag->carrier)
	{
		l_State |= Planner::AgentFlagAtBase;
	}
	if(!m_game->team->flag->carrier &&
		m_game->team->flag->position != m_game->team->flagSpawnLocation)
	{
		l_State |= Planner::AgentFlagDropped;
	}
	if(m_game->enemyTeam->flag->position == m_game->enemyTeam->flagSpawnLocation
		&& !m_game->enemyTeam->flag->carrier)
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
	// TODO : Call the Navigator for the path
	switch (in_Action)
	{
		case Planner::GetEnemyFlag:
		{
			std::cout << in_Bot->name << " : GetEnemyFlag" << std::endl;
			issue(new ChargeCommand(in_Bot->name, m_game->enemyTeam->flag->position));
			break;
		}
		case Planner::WaitEnemyBase:
		{
			// If far -> attack toward enemy base
			if(sqrt(pow(m_game->enemyTeam->flagScoreLocation.x - in_Bot->position->x, 2) + 
				pow(m_game->enemyTeam->flagScoreLocation.y - in_Bot->position->y, 2)) > 0.f)
			{
				std::cout << in_Bot->name << " : WaitEnemyBase/Attack" << std::endl;
				issue(new AttackCommand(in_Bot->name, m_game->enemyTeam->flagScoreLocation, m_game->enemyTeam->flagScoreLocation));
			}
			else // defend/patrol
			{
				std::cout << in_Bot->name << " : WaitEnemyBase/Defend" << std::endl;
				DefendCommand::FacingDirectionVector l_FacingDirections;
				l_FacingDirections.push_back(std::make_pair(*(in_Bot->facingDirection), 1.f));
				l_FacingDirections.push_back(std::make_pair(Vector2(in_Bot->facingDirection->x, -in_Bot->facingDirection->y), 1.f));
				l_FacingDirections.push_back(std::make_pair(Vector2(-in_Bot->facingDirection->x, -in_Bot->facingDirection->y), 1.f));
				l_FacingDirections.push_back(std::make_pair(-Vector2(in_Bot->facingDirection->x, in_Bot->facingDirection->y), 1.f));
				issue(new DefendCommand(in_Bot->name, l_FacingDirections));
			}
			break;
		}
		case Planner::KillFlagCarrier:
		{
			// TODO : Redo to intercept flag carrier
			std::cout << in_Bot->name << " : KillFlagCarrier" << std::endl;
			if(m_game->team->flag->carrier)
				issue(new AttackCommand(in_Bot->name, m_game->team->flag->position, m_game->team->flag->position));
			break;
		}
		case Planner::Defend:
		{
			if(m_game->team->flag->carrier) // Own flag is in enemies' hand
			{
				if(sqrt(pow(m_game->enemyTeam->flagScoreLocation.x - in_Bot->position->x, 2) + 
					pow(m_game->enemyTeam->flagScoreLocation.y - in_Bot->position->y, 2)) > 0.f)
				{
					std::cout << in_Bot->name << " : Defend/ChargeEnemy " << std::endl;
					issue(new ChargeCommand(in_Bot->name, m_game->enemyTeam->flagScoreLocation));
				}
				else
				{
					std::cout << in_Bot->name << " : Defend/CampEnemy" << std::endl;
					DefendCommand::FacingDirectionVector l_FacingDirections;
					l_FacingDirections.push_back(std::make_pair(*(in_Bot->facingDirection), 1.f));
					l_FacingDirections.push_back(std::make_pair(Vector2(in_Bot->facingDirection->x, -in_Bot->facingDirection->y), 1.f));
					l_FacingDirections.push_back(std::make_pair(Vector2(-in_Bot->facingDirection->x, -in_Bot->facingDirection->y), 1.f));
					l_FacingDirections.push_back(std::make_pair(-Vector2(in_Bot->facingDirection->x, in_Bot->facingDirection->y), 1.f));
					issue(new DefendCommand(in_Bot->name, l_FacingDirections));
				}
			}
			else // Own flag is at our base
			{
				if(sqrt(pow(m_game->team->flagSpawnLocation.x - in_Bot->position->x, 2) + 
					pow(m_game->team->flagSpawnLocation.y - in_Bot->position->y, 2)) > 0.f)
				{
					std::cout << in_Bot->name << " : Defend/GetBack" << std::endl;
					issue(new AttackCommand(in_Bot->name, m_game->team->flagSpawnLocation, m_game->team->flagSpawnLocation));
				}
				else
				{
					std::cout << in_Bot->name << " : Defend/DefendBase" << std::endl;
					DefendCommand::FacingDirectionVector l_FacingDirections;
					l_FacingDirections.push_back(std::make_pair(*(in_Bot->facingDirection), 1.f));
					l_FacingDirections.push_back(std::make_pair(Vector2(in_Bot->facingDirection->x, -in_Bot->facingDirection->y), 1.f));
					l_FacingDirections.push_back(std::make_pair(Vector2(-in_Bot->facingDirection->x, -in_Bot->facingDirection->y), 1.f));
					l_FacingDirections.push_back(std::make_pair(-Vector2(in_Bot->facingDirection->x, in_Bot->facingDirection->y), 1.f));
					issue(new DefendCommand(in_Bot->name, l_FacingDirections));
				}
			}
			break;
		}
		case Planner::SupportFlagCarrier:
		{
			// TODO : Better translation to low level instruction once the Navigator is done
			std::cout << in_Bot->name << " : SupportFlagCarrier" << std::endl;
			if(m_game->team->flag->carrier)
				issue(new AttackCommand(in_Bot->name, m_game->team->flagScoreLocation, m_game->team->flag->carrier->position));
			break;
		}
		case Planner::ReturnToBase:
		{
			std::cout << in_Bot->name << " : ReturnToBase" << std::endl;
			issue(new ChargeCommand(in_Bot->name, m_game->team->flagScoreLocation));
			break;
		}
		default:
		{
			break;
		}
	}
}
