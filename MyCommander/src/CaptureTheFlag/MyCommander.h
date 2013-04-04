#ifndef MY_COMMANDER_H
#define MY_COMMANDER_H

#include <string>

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
	std::map<BotInfo*, std::vector<std::shared_ptr<Navigator::Node>>> m_BotsAbstractPaths;
	std::map<BotInfo*, unsigned> m_BotsNodeIndex;
	std::map<BotInfo*, Planner::Actions> m_BotLastAction;

private:
	Planner::State GetBotState(const BotInfo* in_Bot);
	void CompletePath(BotInfo* in_Bot);
	void ActionToCommand(const Planner::Actions in_Action, BotInfo* in_Bot);
	std::vector<Vector2> ComputePathBeginning(BotInfo * in_Bot, const Vector2 & in_Start, const Vector2 & in_Goal);
	
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
