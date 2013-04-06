#ifndef MY_COMMANDER_H
#define MY_COMMANDER_H

#include <string>

#include <boost/optional.hpp>

#include "api\Commander.h"

#include "Navigator.h"
#include "Planner.h"

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* MyCommander
*/

class MyCommander : public Commander
{
private:
	static const double M_TICKTIME;
	static const std::string M_QVALUESFILE;
	static const std::string M_GETFLAGSTR;
	static const std::string M_DEFENDSTR;
	static const std::string M_RETURNSTR;
	static const std::string M_WAITSTR;
	static const std::string M_KILLSTR;
	static const std::string M_SUPPORTSTR;

	double m_TimeSpent;	
	Navigator m_Navigator;
	Planner m_Planner;
	boost::chrono::high_resolution_clock::time_point m_Start;
	std::map<std::string, std::vector<std::shared_ptr<Navigator::Node>>> m_BotsAbstractPaths;
	std::map<std::string, unsigned> m_BotsNodeIndex;
	std::map<std::string, Planner::Actions> m_BotLastAction;

private:
	Planner::State GetBotState(const BotInfo* in_Bot);
	void CompletePath(BotInfo* in_Bot);
	void ActionToCommand(const Planner::Actions in_Action, BotInfo* in_Bot);
	std::vector<Vector2> ComputePathBeginning(BotInfo * in_Bot, const Vector2 & in_Start, const Vector2 & in_Goal);
	
	boost::optional<Vector2> GetBestLookAt(const BotInfo* in_BotInfo) const;

	void CommandGetEnemyFlag(BotInfo* in_Bot);
	void CommandWaitEnemyBase(BotInfo* in_Bot);
	void CommandKillFlagCarrier(BotInfo* in_Bot);
	void CommandDefend(BotInfo* in_Bot);
	void CommandSupportFlagCarrier(BotInfo* in_Bot);
	void CommandReturnToBase(BotInfo* in_Bot);

public:
	MyCommander() { }

	virtual std::string getName() const;
    virtual void initialize();
    virtual void tick();
    virtual void shutdown();
	void Reset();
};

#endif // MY_COMMANDER_H
