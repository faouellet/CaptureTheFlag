#ifndef COMMANDS_H
#define COMMANDS_H

#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <vector>
#include <map>
#include <utility>
#include <boost/optional.hpp>

#include "Vector2.h"

struct Command
{
	// NB : Added {} so it could link with my test project. It seems that
	// giving this destructor a body in another file was the source of the error.
	virtual ~Command() = 0 {}
};


/**
 * Commands a bot to defend its current position.
 */
struct DefendCommand : public Command
{
    typedef std::vector< std::pair<Vector2, float> > FacingDirectionVector;

    /**
     * Instructs the bot to defend facing in its current facing direction.
     * @param botId The bot being ordered.
     * @param description A description of the intention of the bot. This can be optional displayed in the gui next to the bot label.
     */
    DefendCommand(const std::string& botId, const std::string description = "")
    :   m_botId(botId)
    ,   m_facingDirections()
    ,   m_description(description)
    {
    }

    /**
     * Instructs the bot to defend facing in a specified facing direction.
     * @param botId The bot being ordered.
     * @param facingDirection The facing direction of the bot.
     * @param description A description of the intention of the bot. This can be optional displayed in the gui next to the bot label.
     */
    DefendCommand(const std::string& botId, const Vector2& facingDirection, const std::string description = "")
    :   m_botId(botId)
    ,   m_facingDirections(FacingDirectionVector())
    ,   m_description(description)
    {
        m_facingDirections->push_back(std::make_pair(facingDirection, 0.0f));
    }

	DefendCommand(const std::string& botId, Vector2&& facingDirection, const std::string description = "")
    :   m_botId(botId)
    ,   m_facingDirections(FacingDirectionVector())
    ,   m_description(description)
    {
        m_facingDirections->push_back(std::make_pair(facingDirection, 0.0f));
    }

    /**
     * Instructs the bot to defend changing between different facing directions.
     * @param botId The bot being ordered.
     * @param facingDirection A vector of (direction, time) pairs. The bot will alternate the different facing directions, pausing at each one for the specified time.
     * @param description A description of the intention of the bot. This can be optional displayed in the gui next to the bot label.
     */
    DefendCommand(const std::string& botId, const FacingDirectionVector& facingDirections, const std::string description = "")
    :   m_botId(botId)
    ,   m_facingDirections(facingDirections)
    ,   m_description(description)
    {
    }

	DefendCommand(const std::string& botId, FacingDirectionVector&& facingDirections, const std::string description = "")
    :   m_botId(botId)
    ,   m_facingDirections(facingDirections)
    ,   m_description(description)
    {
    }

    std::string m_botId;
    boost::optional<FacingDirectionVector> m_facingDirections;
    std::string m_description;
};

/**
 * Commands a bot to run to a specified position without attacking visible enemies.
 */
struct MoveCommand : public Command
{
    /**
     * @param botId The bot being ordered.
     * @param target The target location that the bot will move to.
     * @param description A description of the intention of the bot. This can be optional displayed in the gui next to the bot label.
     */
    MoveCommand(const std::string& botId, const Vector2& target, const std::string description = "")
        :   m_botId(botId)
        ,   m_target(1,target)
        ,   m_description(description)
    {
    }

	MoveCommand(const std::string& botId, Vector2&& target, const std::string description = "")
        :   m_botId(botId)
        ,   m_target(1,target)
        ,   m_description(description)
    {
    }

    MoveCommand(const std::string& botId, const std::vector<Vector2>& target, const std::string description = "")
        :   m_botId(botId)
        ,   m_target(target)
        ,   m_description(description)
    {
    }

	MoveCommand(const std::string& botId, std::vector<Vector2>&& target, const std::string description = "")
        :   m_botId(botId)
        ,   m_target(target)
        ,   m_description(description)
    {
    }

    std::string m_botId;
    std::vector<Vector2> m_target;
    std::string m_description;
};


/**
 * Commands a bot to attack a specified position. If an enemy bot is seen by this bot, it will be attacked.
 */
struct AttackCommand : public Command
{
    /**
     * @param botId The bot being ordered.
     * @param target The target location that the bot will attack.
     * @param lookAt An optional position which the bot should look at while moving.
     * @param description A description of the intention of the bot. This can be optional displayed in the gui next to the bot label.
     */
    AttackCommand(const std::string& botId, const Vector2& target, const boost::optional<Vector2>& lookAt, const std::string description = "")
    :   m_botId(botId)
    ,   m_target(1,target)
    ,   m_lookAt(lookAt)
    ,   m_description(description)
    {
    }

	AttackCommand(const std::string& botId, Vector2 && target, const boost::optional<Vector2>& lookAt, const std::string description = "")
    :   m_botId(botId)
    ,   m_target(1,target)
    ,   m_lookAt(lookAt)
    ,   m_description(description)
    {
    }

    AttackCommand(const std::string& botId, const std::vector<Vector2>& waypoints, const boost::optional<Vector2>& lookAt, const std::string description = "")
        :   m_botId(botId)
        ,   m_target(waypoints)
        ,   m_lookAt(lookAt)
        ,   m_description(description)
    {
    }

	AttackCommand(const std::string& botId, std::vector<Vector2>&& waypoints, const boost::optional<Vector2>& lookAt, const std::string description = "")
        :   m_botId(botId)
        ,   m_target(waypoints)
        ,   m_lookAt(lookAt)
        ,   m_description(description)
    {
    }

	virtual ~AttackCommand() {}

    std::string m_botId;
    std::vector<Vector2> m_target;
    boost::optional<Vector2> m_lookAt;
    std::string m_description;
};


/**
 * Commands a bot to attack a specified position at a running pace. This is faster than Attack but incurs an additional firing delay penalty.
 */
struct ChargeCommand : public Command
{
    /**
     * @param botId The bot being ordered.
     * @param target The target location that the bot will charge to.
     * @param description A description of the intention of the bot. This can be optional displayed in the gui next to the bot label.
     */
    ChargeCommand(const std::string& botId, const Vector2& target, const std::string description = "")
        :   m_botId(botId)
        ,   m_target(1,target)
        ,   m_description(description)
    {
    }

	ChargeCommand(const std::string& botId, Vector2&& target, const std::string description = "")
        :   m_botId(botId)
        ,   m_target(1,target)
        ,   m_description(description)
    {
    }

    ChargeCommand(const std::string& botId, const std::vector<Vector2>& waypoints, const std::string description = "")
        :   m_botId(botId)
        ,   m_target(waypoints)
        ,   m_description(description)
    {
    }

	ChargeCommand(const std::string& botId, std::vector<Vector2>&& waypoints, const std::string description = "")
        :   m_botId(botId)
        ,   m_target(waypoints)
        ,   m_description(description)
    {
    }

    std::string m_botId;
    std::vector<Vector2> m_target;
    std::string m_description;
};


#endif // COMMANDS_H
