#ifndef GAMEINFO_H
#define GAMEINFO_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <utility>

#include <boost/optional.hpp>

#include "Vector2.h"

struct LevelInfo;
struct GameInfo;
struct TeamInfo;
struct BotInfo;
struct FlagInfo;
struct MatchInfo;
struct MatchCombatEvent;


/**
 * Provides information about the level the game is played in
 */
struct LevelInfo
{
    int width;                                                         // The width of the game world
    int height;                                                        // The height of the game world
    std::unique_ptr<float[]> blockHeights;                             // A width x height array showing the height of the block at each position in the world
                                                                       // indexing is based on x + y * width
    std::vector<std::string> teamNames;                                // A list of the team names supported by this level.
    std::map<std::string, Vector2> flagSpawnLocations;                 // The map of team name the spawn location of the team's flag
    std::map<std::string, Vector2> flagScoreLocations;                 // The map of team name the location the flag must be taken to score
    std::map<std::string, std::pair<Vector2, Vector2> > botSpawnAreas; // The map of team name the extents of each team's bot spawn area

    float characterRadius;                                             // The radius of each character, used to determine the passability region around blocks
    std::unique_ptr<float[]> fieldOfViewAngles;                        // This is a list of field of view angles for each of the states a bot can be in
    float firingDistance;                                              // The maximum firing distance of the bots
    float walkingSpeed;                                                // The walking speed of the bots
    float runningSpeed;                                                // The running speed of the bots

    float gameLength;                                                  // The length (seconds) of the game.
    float initializationTime;                                          // The time (seconds) allowed for bot initialization before the start of the game.
    float respawnTime;                                                 // The time (seconds) between bot respawns.

    bool findRandomFreePositionInBox(Vector2& result, const Vector2& min, const Vector2& max );
    bool findNearestFreePosition(Vector2& result, const Vector2& position);
};


/** 
 * All of the filtered read-only information about the current game state.
 * This shouldn't be modified. Modifying it will only hurt yourself.
 * Updated each frame to show the current known information about the world.
 */
struct GameInfo
{
    std::unique_ptr<MatchInfo> match;                                  // The MatchInfo describing the current match
    std::map<std::string, std::unique_ptr<TeamInfo> > teams;           // Map of team names to TeamInfo
    TeamInfo* team;                                                    // The team this commander is controlling
    TeamInfo* enemyTeam;                                               // The enemy team for this commander
    std::map<std::string, std::unique_ptr<BotInfo> > bots;             // Map of bot name to BotInfo
    std::map<std::string, std::unique_ptr<FlagInfo> > flags;           // Map of flag name to BotInfo

    // updated by the client wrapper before commander tick
    std::vector<BotInfo*> bots_alive;                                  // List of bots in the commander's team that are alive
    std::vector<BotInfo*> bots_available;                              // List of bots in the commander's team that are alive and idle
    std::vector<FlagInfo*> enemyFlags;                                 // List of flags that don't belong to this commander's team
};


/**
 * Information about the current team including ids of all of the members of the team
 */
struct TeamInfo
{
    std::string name;                                                  // The name of the team
    std::vector<BotInfo*> members;                                     // The bots that are members of this team
    FlagInfo* flag;                                                    // The flag for this team
    Vector2 flagSpawnLocation;                                         // The location where this team's flag is spawned
    Vector2 flagScoreLocation;                                         // The location where this team must take enemy flags to score
    std::pair<Vector2, Vector2> botSpawnArea;                          // The area in which this team's bots are spawned
};


/**
 * Information about each of the flags.
 * The positions of all flags are always known.
 * If a flag is being carried the carrier is always known
 */
struct FlagInfo
{
    std::string name;                                                  // The name of this flag
    TeamInfo* team;                                                    // The team that owns this flag
    Vector2 position;                                                  // The position of this flag
    BotInfo* carrier;                                                  // The bot carrying this flag, nullptr if the flag is not being carried
    float respawnTimer;                                                // The time remaining until the dropped flag is respawned
};


/**
 * Information that you know about each of the bots.
 * Enemy bots will contain information about the last time they were seen.
 * Friendly bots will contain full information.
 */
struct BotInfo
{
    // possible bot states
    enum State { STATE_UNKNOWN=0, STATE_IDLE = 1, STATE_DEFENDING=2, STATE_MOVING=3, STATE_ATTACKING=4, STATE_CHARGING=5, STATE_SHOOTING=6, STATE_TAKINGORDERS=7, STATE_HOLDING=8, STATE_DEAD=9 };

    std::string name;                                                  // The name of the bot.
    TeamInfo* team;                                                    // The team this bot belongs to.
    boost::optional<float> seenlast;                                   // The time since the bot was last seen (0 if the bot was seen this frame).
    boost::optional<float> health;                                     // The last known health of the bot. (As of CTF 1.7, this will always be set)
    boost::optional<Vector2> position;                                 // The last known position of the bot. (As of CTF 1.7, this will always be set)
    boost::optional<Vector2> facingDirection;                          // The last known facing direction of the bot. (As of CTF 1.7, this will always be set)
    boost::optional<State> state;                                      // The last known state of the bot. (As of CTF 1.7, this will always be set)
    FlagInfo* flag;                                                    // The flag this bot is carrying, nullptr if the bot is not carrying a flag
    std::vector<BotInfo*> visibleEnemies;                              // The list of bots that are visible to this bot
                                                                       // For enemy bots that are not visible this will be an empty list
    std::vector<BotInfo*> seenBy;                                      // The list of bots that are seen by this bot
                                                                       // For enemy bots that are not visible this will be an empty list
};


/**
 * Information about the current match.
 */
struct MatchInfo
{
    std::map<std::string, float> scores;                               // The map of team name to the current score of each team.
    float timeRemaining;                                               // The time (seconds) remaining in this game.
    float timeToNextRespawn;                                           // The time (seconds) until the next bot respawn cycle.
    float timePassed;                                                  // The time (seconds) passed since this game started.
    std::vector<MatchCombatEvent> combatEvents;                        // The list of all events that have occured in this game.
};



/**
 * Information about a particular game event.
 */
struct MatchCombatEvent
{
    enum Type { TYPE_NONE = 0, TYPE_KILLED, TYPE_FLAG_PICKEDUP, TYPE_FLAG_DROPPED, TYPE_FLAG_CAPTURED, TYPE_FLAG_RESTORED, TYPE_RESPAWN };

    Type type;                                                         // The type of the event
    float time;                                                        // The time (seconds since the start of the game) that the event occurred

    struct KilledEventData
    {
        BotInfo* instigator;                                           // The bot that killed the other bot.
        BotInfo* subject;                                              // The bot that was killed.
    };
    
    struct FlagPickedUpEventData
    {
        BotInfo* instigator;                                           // The bot that picked the flag up.
        FlagInfo* subject;                                             // The flag that was picked up.
    };

    struct FlagDroppedEventData
    {
        BotInfo* instigator;                                           // The bot that dropped the flag.
        FlagInfo* subject;                                             // The flag that was dropped.
    };

    struct FlagCapturedEventData
    {
        BotInfo* instigator;                                           // The bot that captured the flag. (took it to the scoring location)
        FlagInfo* subject;                                             // The flag that was captured.
    };

    struct FlagRestoredEventData
    {
        FlagInfo* subject;                                             // The flag that was restored.
    };

    struct RespawnEventData
    {
        BotInfo* subject;                                              // The bot that was spawned.
    };

    union                                                              // A union containing the event type specific data.
    {                                                                  // eg. if (event.type == MatchCombatEvent::TYPE_KILLED)
        KilledEventData killedEventData;                               //     {
        FlagPickedUpEventData flagPickupEventData;                     //         BotInfo* killed = event.killedEventData.instigator;
        FlagDroppedEventData flagDroppedEventData;                     //         BotInfo* died = event.killedEventData.subject;
        FlagCapturedEventData flagCapturedEventData;                   //     }
        FlagRestoredEventData flagRestoredEventData;                   //
        RespawnEventData respawnEventData;                             //
    };
};

#endif // GAMEINFO_H
