#ifndef MY_COMMANDER_H
#define MY_COMMANDER_H

#include <string>

#include "api\Commander.h"

#include "Navigator.h"
#include "Planner.h"

class MyCommander : public Commander
{
private:
	Planner m_Planner;
	Navigator m_Navigator;
	std::map<BotInfo*, Planner::Action> m_CurrentOrders;
	std::map<BotInfo*, Planner::State> m_PreviousState;

private:
	Planner::State GetBotState(const BotInfo* in_Bot);

public:
	virtual std::string getName() const;
    virtual void initialize();
    virtual void tick();
    virtual void shutdown();

};

#endif // MY_COMMANDER_H
