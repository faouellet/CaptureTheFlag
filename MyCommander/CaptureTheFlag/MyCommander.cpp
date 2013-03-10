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
}


void MyCommander::tick()
{
    // TODO : Transform the level info in a list of valid states for the Planner
	// NB : Only for the available bots. Consequence, move will be done a node at a time for quick reaction in case of enemy attack
    // TODO : Ask the Planner what the bots should do
	// TODO : Breakdown the high level actions into Commands
    // TODO : Ask directions to the Navigator for the bots that will be moving
    // TODO : Process continuous task with the time left
}


void MyCommander::shutdown()
{
    // TODO : ?
}

Planner::State MyCommander::GetBotState(const BotInfo* in_Bot)
{
	// TODO
	return Planner::State();
}
