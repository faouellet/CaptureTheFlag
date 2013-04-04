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

#include "Heuristics.h"

REGISTER_COMMANDER(MyCommander);

const double MyCommander::M_TICKTIME = 80.0;
const std::string MyCommander::M_QVALUESFILE = "QValues.json";
const std::string MyCommander::M_GETFLAGSTR = "GetEnemyFlag";
const std::string MyCommander::M_DEFENDSTR = "Defend";
const std::string MyCommander::M_RETURNSTR = "ReturnToBase";
const std::string MyCommander::M_WAITSTR = "WaitEnemyBase";
const std::string MyCommander::M_KILLSTR = "KillFlagCarrier";
const std::string MyCommander::M_SUPPORTSTR = "SupportFlagCarrier";

std::string MyCommander::getName() const
{
    return "MyCommander";
}

void MyCommander::initialize()
{
	std::for_each(m_game->team->members.begin(), m_game->team->members.end(), 
			[this](BotInfo* in_BotInfo)
		{
			m_BotsAbstractPaths[in_BotInfo] = std::vector<std::shared_ptr<Navigator::Node>>(0);
			m_BotsNodeIndex[in_BotInfo] = 0;
			m_BotLastAction[in_BotInfo] = Planner::None;
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
		// Don't issue orders to bots not finished with their abstract path
		if(m_BotsNodeIndex[l_Bot] && m_BotsNodeIndex[l_Bot] < m_BotsAbstractPaths[l_Bot].size() - 1)
		{
			CompletePath(l_Bot);
		}
		else
		{
			m_BotsAbstractPaths[l_Bot].clear();
			m_BotsNodeIndex[l_Bot] = 0;
			Planner::State l_CurrentState = GetBotState(l_Bot);
			Planner::Actions l_Action = m_Planner.GetNextAction(l_Bot, l_CurrentState, m_game->match->timePassed, m_game->match->combatEvents);
			ActionToCommand(l_Action, l_Bot);
		}
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

void MyCommander::ActionToCommand(const Planner::Actions in_Action, BotInfo* in_Bot)
{
	switch (in_Action)
	{
		case Planner::GetEnemyFlag:
			CommandGetEnemyFlag(in_Bot);
			break;
		case Planner::WaitEnemyBase:
			CommandWaitEnemyBase(in_Bot);
			break;
		case Planner::KillFlagCarrier:
			CommandKillFlagCarrier(in_Bot);			
			break;
		case Planner::Defend:
			CommandDefend(in_Bot);
			break;
		case Planner::SupportFlagCarrier:
			CommandSupportFlagCarrier(in_Bot);
			break;
		case Planner::ReturnToBase:
			CommandReturnToBase(in_Bot);
			break;
		default:
			break;
	}
}

void MyCommander::CommandGetEnemyFlag(BotInfo* in_Bot)
{
	std::vector<Vector2> l_Path(ComputePathBeginning(in_Bot,
	// NOTE : The bot position is supposed to always be set as per the SDK documentation, but in reality it isn't so I have to check
	in_Bot->position ? *in_Bot->position : m_game->team->botSpawnArea.first, m_game->enemyTeam->flag->position));

	if(l_Path.size())
	{
		issue(new ChargeCommand(in_Bot->name, l_Path, M_GETFLAGSTR));
		m_BotsNodeIndex[in_Bot]+=2;
	}
	m_BotLastAction[in_Bot] = Planner::GetEnemyFlag;
}

void MyCommander::CommandWaitEnemyBase(BotInfo* in_Bot)
{
	if(in_Bot->position->squaredDistance(m_game->team->flag->position) > 1.f)
	{
		std::vector<Vector2> l_Path(ComputePathBeginning(in_Bot, 
			m_game->enemyTeam->flagScoreLocation, m_game->enemyTeam->flagScoreLocation));
		if(l_Path.size())
		{
			issue(new AttackCommand(in_Bot->name, l_Path, nullptr, M_WAITSTR));
			m_BotsNodeIndex[in_Bot]+=2;
		}				
	}
	else
	{
		DefendCommand::FacingDirectionVector l_FacingDirections;
		l_FacingDirections.push_back(std::make_pair(*(in_Bot->facingDirection), 1.f));
		l_FacingDirections.push_back(std::make_pair(Vector2(in_Bot->facingDirection->x, -in_Bot->facingDirection->y), 1.f));
		l_FacingDirections.push_back(std::make_pair(Vector2(-in_Bot->facingDirection->x, -in_Bot->facingDirection->y), 1.f));
		l_FacingDirections.push_back(std::make_pair(-Vector2(in_Bot->facingDirection->x, in_Bot->facingDirection->y), 1.f));
		issue(new DefendCommand(in_Bot->name, l_FacingDirections, M_WAITSTR));
	}
	m_BotLastAction[in_Bot] = Planner::WaitEnemyBase;
}

void MyCommander::CommandKillFlagCarrier(BotInfo* in_Bot)
{
	if(m_game->team->flag->carrier)
	{
		std::vector<std::shared_ptr<Navigator::Node>> l_EnemyAbstractPath(m_Navigator.ComputeAbstractPath(
			*m_game->team->flag->carrier->position,
			m_game->enemyTeam->flagScoreLocation, TrivialHeuristic()));

		std::vector<std::shared_ptr<Navigator::Node>> l_BotAbstractPath(m_Navigator.ComputeAbstractPath(
			*in_Bot->position,
			m_game->team->flagScoreLocation, TrivialHeuristic()));

		std::reverse(l_EnemyAbstractPath.begin(), l_EnemyAbstractPath.end());
		l_BotAbstractPath.insert(l_BotAbstractPath.end(), l_EnemyAbstractPath.begin()+1, l_EnemyAbstractPath.end());

		m_BotsAbstractPaths[in_Bot] = l_BotAbstractPath;

		std::vector<Vector2> l_ConcretePath(m_Navigator.ComputeConcretePath(
				std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]]), 
				std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]+1]), TrivialHeuristic()));

		issue(new ChargeCommand(in_Bot->name, l_ConcretePath, M_KILLSTR));

	}
	m_BotLastAction[in_Bot] = Planner::KillFlagCarrier;
}

void MyCommander::CommandDefend(BotInfo* in_Bot)
{
	if(in_Bot->position->squaredDistance(m_game->team->flag->position) > 1.f)
	{
		std::vector<Vector2> l_Path(ComputePathBeginning(in_Bot, 
			*in_Bot->position, m_game->team->flag->position));
		if(l_Path.size())
		{
			issue(new ChargeCommand(in_Bot->name, l_Path, M_DEFENDSTR));
			m_BotsNodeIndex[in_Bot]+=2;
		}
	}
	else
	{
		DefendCommand::FacingDirectionVector l_FacingDirections;
		l_FacingDirections.push_back(std::make_pair(*(in_Bot->facingDirection), 1.f));
		l_FacingDirections.push_back(std::make_pair(Vector2(in_Bot->facingDirection->x, -in_Bot->facingDirection->y), 1.f));
		l_FacingDirections.push_back(std::make_pair(Vector2(-in_Bot->facingDirection->x, -in_Bot->facingDirection->y), 1.f));
		l_FacingDirections.push_back(std::make_pair(-Vector2(in_Bot->facingDirection->x, in_Bot->facingDirection->y), 1.f));
		issue(new DefendCommand(in_Bot->name, l_FacingDirections, M_DEFENDSTR));
	}
	m_BotLastAction[in_Bot] = Planner::Defend;
}

void MyCommander::CommandSupportFlagCarrier(BotInfo* in_Bot)
{
	if(m_game->enemyTeam->flag->carrier)
	{
		std::vector<std::shared_ptr<Navigator::Node>> l_BotAbstractPath(m_Navigator.ComputeAbstractPath(
			*in_Bot->position,
			m_BotsAbstractPaths[m_game->enemyTeam->flag->carrier][m_BotsAbstractPaths[m_game->enemyTeam->flag->carrier].size()/2]->Position, 
			TrivialHeuristic()));

		std::vector<std::shared_ptr<Navigator::Node>> l_ReturnPath(m_Navigator.ComputeAbstractPath(
			m_BotsAbstractPaths[m_game->enemyTeam->flag->carrier][m_BotsAbstractPaths[m_game->enemyTeam->flag->carrier].size()/2]->Position,
			m_game->team->flagScoreLocation,
			TrivialHeuristic()));

		l_BotAbstractPath.insert(l_BotAbstractPath.end(), l_ReturnPath.begin()+1, l_ReturnPath.end());

		std::vector<Vector2> l_ConcretePath(m_Navigator.ComputeConcretePath(
				std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]]), 
				std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]+1]),
				TrivialHeuristic()));

		issue(new AttackCommand(in_Bot->name, l_ConcretePath, nullptr, M_SUPPORTSTR));
	}
	m_BotLastAction[in_Bot] = Planner::SupportFlagCarrier;
}

void MyCommander::CommandReturnToBase(BotInfo* in_Bot)
{
	std::vector<Vector2> l_Path(ComputePathBeginning(in_Bot, 
		*in_Bot->position, m_game->team->flagScoreLocation));
	if(l_Path.size())
	{
		issue(new ChargeCommand(in_Bot->name, l_Path, M_RETURNSTR));
		m_BotsNodeIndex[in_Bot]+=2;
	}
	m_BotLastAction[in_Bot] = Planner::ReturnToBase;
}

std::vector<Vector2> MyCommander::ComputePathBeginning(BotInfo * in_Bot, const Vector2 & in_Start, const Vector2 & in_Goal)
{
	std::cout << in_Bot->name << " Start:" << in_Start << " Goal: " << in_Goal << std::endl;

	m_BotsAbstractPaths[in_Bot] = m_Navigator.ComputeAbstractPath(in_Start, in_Goal, TrivialHeuristic());

	if(m_BotsAbstractPaths[in_Bot].size())
	{
		Vector2 l_Goal = m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]+1]->Position;
		std::vector<Vector2> l_ConcretePath(m_Navigator.ComputeConcretePath(
				std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]]), 
				std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]+1]), TrivialHeuristic()));

		if(l_ConcretePath.empty())
			l_ConcretePath.push_back(in_Goal);
		return l_ConcretePath;
	}
	return std::vector<Vector2>();
}

void MyCommander::CompletePath(BotInfo* in_Bot)
{
	std::string l_Intention;
	switch (m_BotLastAction.at(in_Bot))
	{
		case Planner::GetEnemyFlag:
			l_Intention = M_GETFLAGSTR;
			break;
		case Planner::Defend:
			l_Intention = M_DEFENDSTR;
			break;
		case Planner::ReturnToBase:
			l_Intention = M_RETURNSTR;
			break;
		case Planner::WaitEnemyBase:
			l_Intention = M_WAITSTR;
			break;
		case Planner::KillFlagCarrier:
			l_Intention = M_KILLSTR;
			break;
		case Planner::SupportFlagCarrier:
			l_Intention = M_RETURNSTR;
			break;
		default:
			l_Intention = "";
			break;
	}

	switch (m_BotLastAction.at(in_Bot))
	{
		case Planner::GetEnemyFlag:
		case Planner::Defend:
		case Planner::ReturnToBase:
		{
			if(m_BotsNodeIndex[in_Bot] == m_BotsAbstractPaths[in_Bot].size()-1)
			{
				issue(new ChargeCommand(in_Bot->name, m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]]->Position, l_Intention));
			}
			else
			{
				Vector2 l_Goal = m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]+1]->Position;
				std::vector<Vector2> l_ConcretePath( m_Navigator.ComputeConcretePath(
							std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]]), 
							std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]+1]), 
							TrivialHeuristic()));

				if(l_ConcretePath.size())
					issue(new ChargeCommand(in_Bot->name, l_ConcretePath, l_Intention));
				else
					issue(new ChargeCommand(in_Bot->name, l_Goal, l_Intention));

				m_BotsNodeIndex[in_Bot]+=2;
			}
			break;
		}
		case Planner::KillFlagCarrier:
		{
			if((abs(in_Bot->position->x - m_game->enemyTeam->flagScoreLocation.x) 
				+ abs(in_Bot->position->y - m_game->enemyTeam->flagScoreLocation.y)) < 1.f)
			{
				issue(new AttackCommand(in_Bot->name, m_Navigator.ComputeConcretePath(
					std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]]),
					std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]+1]), 
					TrivialHeuristic()), nullptr, l_Intention));
			}
			else
			{
				issue(new ChargeCommand(in_Bot->name, m_Navigator.ComputeConcretePath(
					std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]]),
					std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]+1]), TrivialHeuristic()), l_Intention));
			}
			m_BotsNodeIndex[in_Bot]+=2;
			break;
		}
		case Planner::SupportFlagCarrier:
		case Planner::WaitEnemyBase:		
		{
			issue(new AttackCommand(in_Bot->name, m_Navigator.ComputeConcretePath(
				std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]]),
				std::move(m_BotsAbstractPaths[in_Bot][m_BotsNodeIndex[in_Bot]+1]), TrivialHeuristic()), nullptr, l_Intention));
			break;
		}
	}
}

void MyCommander::Reset()
{
	m_Navigator.Reset();
	m_Planner.Reset();
}
