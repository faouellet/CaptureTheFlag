/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*/

#include "MyCommander.h"

#include <cassert>
#include <iostream>
#include <fstream>

#include "api/CommanderFactory.h"
#include "api/Commands.h"
#include "api/GameInfo.h"

REGISTER_COMMANDER(MyCommander);

const double MyCommander::M_TICKTIME = 80.0;
const std::string MyCommander::M_QVALUESFILE = "QValues.json";

std::string MyCommander::getName() const
{
    return "MyCommander";
}

void MyCommander::initialize()
{
	std::for_each(m_game->team->members.begin(), m_game->team->members.end(), 
			[this](const BotInfo* in_BotInfo)
		{
			m_BotsAbstractPaths[in_BotInfo] = std::vector<std::shared_ptr<Navigator::Node>>();
			m_BotsNodeIndex[in_BotInfo] = 0;
		});

	m_Navigator.Init(m_level->blockHeights, m_level->height, m_level->width);
#ifdef _TRAIN
	m_Planner.Init(m_game, false);
#else
	m_Planner.Init(m_game, true);
#endif
}

void MyCommander::tick()
{
	m_TimeSpent = 0;
	m_Start = boost::chrono::high_resolution_clock::now();

	for (size_t i = 0; i< m_game->bots_available.size(); ++i)
	{
		auto l_Bot = m_game->bots_available[i];
		Planner::State l_CurrentState = GetBotState(l_Bot);
		Planner::Actions l_Action = m_Planner.GetNextAction(l_Bot, l_CurrentState, m_game->match->timePassed, m_game->match->combatEvents);
		ActionToCommand(l_Action, l_Bot);
	}

	m_Navigator.ProcessClusters(M_TICKTIME 
		- boost::chrono::duration<double, boost::milli>(boost::chrono::high_resolution_clock::now()-m_Start).count());
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
	switch (in_Action)
	{
		case Planner::GetEnemyFlag:
		{
			std::cout << in_Bot->name << " : GetEnemyFlag" << std::endl;
			// NOTE : The bot position is supposed to always be set as per the SDK documentation, but it isn't so I have to compensate
			std::vector<std::shared_ptr<Navigator::Node>> l_AbstractPath(m_Navigator.ComputeAbstractPath(
				in_Bot->position ? *in_Bot->position : m_game->team->botSpawnArea.first, m_game->enemyTeam->flag->position));
			if(l_AbstractPath.size())
			{
				std::vector<Vector2> l_ConcretePath(m_Navigator.ComputeConcretePath(std::move(l_AbstractPath[0]), 
					std::move(l_AbstractPath[1])));
			}

			issue(new ChargeCommand(in_Bot->name, m_game->enemyTeam->flag->position));

			//issue(new ChargeCommand(in_Bot->name, m_game->enemyTeam->flag->position));

			break;
		}
		case Planner::WaitEnemyBase:
		{
			// If far -> attack toward enemy base
			if(in_Bot->position->squaredDistance(m_game->team->flag->position) > 1.f)
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
			std::cout << in_Bot->name << " : KillFlagCarrier" << std::endl;
			if(m_game->team->flag->carrier)
				issue(new AttackCommand(in_Bot->name, m_game->team->flag->position, m_game->team->flag->position));
			break;
		}
		case Planner::Defend:
		{
			if(in_Bot->position->squaredDistance(m_game->team->flag->position) > 1.f)
			{
				std::cout << in_Bot->name << " : Defend/ChargeEnemy " << std::endl;
				issue(new ChargeCommand(in_Bot->name, m_game->team->flag->position));
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
			break;
		}
		case Planner::SupportFlagCarrier:
		{
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

void MyCommander::Reset()
{
	m_Navigator.Reset();
	m_Planner.ResetQValues();
}
