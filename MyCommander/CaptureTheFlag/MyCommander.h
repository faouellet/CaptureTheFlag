#ifndef MY_COMMANDER_H
#define MY_COMMANDER_H

#include <string>

#include "api\Commander.h"

#include "Planner.h"

class MyCommander : public Commander
{
private:
	Planner m_Planner;

private:
	Planner::State GetBotState(const BotInfo* in_Bot);

public:
	MyCommander() { }

	virtual std::string getName() const;
    virtual void initialize();
    virtual void tick();
    virtual void shutdown();
	void ActionToCommand(const Planner::Actions in_Action, const BotInfo* in_Bot);

};

#endif // MY_COMMANDER_H
