#ifndef COMMANDER_H
#define COMMANDER_H

#include <string>
#include <memory>
#include <vector>
#include <utility>

// TODO : How come class forwarding produces incomplete types errors?
// struct Commands;
// struct LevelInfo;
// struct GameInfo;

#include "GameInfo.h"
#include "Commands.h"

/**
 * The base class for Commanders, that give orders to the team members.
 * This class should be inherited from to create your own competition Commander.
 * You must implement `tick(self)` in your custom Commander.
 */
class Commander
{
public:
	virtual ~Commander() {}

    /**
     * Override this to set your commander name.
     */
    virtual std::string getName() const = 0;

    /**
     * Override this function for your own bots.
     * Use this function to setup your bot before the game starts.
     */
    virtual void initialize() {}

    /**
     * Override this function for your own bots.
     * Here you can access all the information in m_level (information about the level being played)
     * and m_game (information about the current game state).
     * You can send commands to your bots using the issue member function
     */
    virtual void tick() = 0;

    /**
     * Override this function for your own bots.
     * Use this function to teardown your bot after the game is over.
     */
    virtual void shutdown() {}

    /**
     * Issue a command for a single bot, with optional arguments depending on the command.
     */
    void issue(const Command* cmd)
    {
        m_commands.push_back(std::unique_ptr<const Command>(cmd));
    }

	/**
	* HACK
	*/
	void setPrivates(std::unique_ptr<GameInfo> game, std::unique_ptr<LevelInfo> level)
	{
		game->team->flag = game->flags["BlueFlag"].get();
		game->enemyTeam->flag = game->flags["RedFlag"].get();
		m_game = std::move(game);
		m_level = std::move(level);
	}

protected:
    std::unique_ptr<LevelInfo> m_level;
    std::unique_ptr<GameInfo> m_game;

private:
    std::vector< std::unique_ptr<const Command> > m_commands;

    friend class NetworkCommanderClient;
};

#endif // COMMANDER_H
