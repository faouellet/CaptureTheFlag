#ifndef JSON_H
#define JSON_H

#include <cassert>
#include <map>
#include <vector>
#include <utility>

#include <json_spirit_reader_template.h>
#include <json_spirit_writer_template.h>

#include "Vector2.h"
#include "GameInfo.h"
#include "Commands.h"
#include "Handshaking.h"

using namespace std;
using namespace json_spirit;

inline BotInfo* toBotInfo(const std::string& name);
inline std::vector<BotInfo*> toBotInfoVector(const json_spirit::mArray& arr);
inline FlagInfo* toFlagInfo(const std::string& name);
inline std::unique_ptr<float[]> toFloatArray(const json_spirit::mArray& arr, int length);
inline std::unique_ptr<float[]> toFloatArray(const json_spirit::mArray& arr, int width, int height);
inline std::vector<MatchCombatEvent> toMatchCombatEventVector(const json_spirit::mArray& arr);
inline std::map<std::string, std::unique_ptr<BotInfo> > toStringBotInfoMap(const json_spirit::mObject& dict);
inline std::map<std::string, std::unique_ptr<FlagInfo> > toStringFlagInfoMap(const json_spirit::mObject& dict);
inline std::map<std::string, float> toStringFloatMap(const json_spirit::mObject& dict);
inline std::map<std::string, std::unique_ptr<TeamInfo> > toStringTeamInfoMap(const json_spirit::mObject& dict);
inline std::map<std::string, Vector2> toStringVector2Map(const json_spirit::mObject& dict);
inline std::map<std::string, std::pair<Vector2, Vector2> > toStringVector2PairMap(const json_spirit::mObject& dict);
inline TeamInfo* toTeamInfo(const std::string& name);
inline Vector2 toVector2(const json_spirit::mArray& arr);
inline std::pair<Vector2, Vector2> toVector2Pair(const json_spirit::mArray& arr);

template <typename T>
inline std::unique_ptr<T> fromJSON(const json_spirit::mValue& value);

template <>
inline std::unique_ptr<ConnectServer> fromJSON<>(const json_spirit::mValue& value)
{
    assert(value.type() == json_spirit::obj_type);
    json_spirit::mObject metaDict = value.get_obj();
    assert(metaDict["__class__"] == "ConnectServer");
    json_spirit::mObject dict = metaDict["__value__"].get_obj();

    std::unique_ptr<ConnectServer> connect(new ConnectServer);
    connect->m_protocolVersion = dict["protocolVersion"].get_str();
    return connect;
}

template <>
inline std::unique_ptr<ConnectClient> fromJSON<>(const json_spirit::mValue& value)
{
    assert(value.type() == json_spirit::obj_type);
    json_spirit::mObject metaDict = value.get_obj();
    assert(metaDict["__class__"] == "ConnectClient");
    json_spirit::mObject dict = metaDict["__value__"].get_obj();

    std::unique_ptr<ConnectClient> connect(new ConnectClient);
    connect->m_commanderName = dict["commanderName"].get_str();
    connect->m_language = dict["language"].get_str();
    return connect;
}

template <>
inline std::unique_ptr<LevelInfo> fromJSON<>(const json_spirit::mValue& value)
{
    assert(value.type() == json_spirit::obj_type);
    json_spirit::mObject metaDict = value.get_obj();
    assert(metaDict["__class__"] == "LevelInfo");
    json_spirit::mObject dict = metaDict["__value__"].get_obj();

    std::unique_ptr<LevelInfo> level(new LevelInfo);
    level->width = dict["width"].get_int();
    level->height = dict["height"].get_int();
    level->blockHeights = toFloatArray(dict["blockHeights"].get_array(), level->width, level->height);
    level->flagSpawnLocations = toStringVector2Map(dict["flagSpawnLocations"].get_obj());
    level->flagScoreLocations = toStringVector2Map(dict["flagScoreLocations"].get_obj());
    level->botSpawnAreas = toStringVector2PairMap(dict["botSpawnAreas"].get_obj());
    level->characterRadius= float(dict["characterRadius"].get_real());
    level->fieldOfViewAngles = toFloatArray(dict["fieldOfViewAngles"].get_array(), 9);
    level->firingDistance = float(dict["firingDistance"].get_real());
    level->walkingSpeed = float(dict["walkingSpeed"].get_real());
    level->runningSpeed = float(dict["runningSpeed"].get_real());
    level->gameLength = float(dict["gameLength"].get_real());
    level->initializationTime = float(dict["initializationTime"].get_real());
    level->respawnTime = float(dict["respawnTime"].get_real());
    return level;
}

template <>
inline std::unique_ptr<TeamInfo> fromJSON<>(const json_spirit::mValue& value)
{
    assert(value.type() == json_spirit::obj_type);
    json_spirit::mObject metaDict = value.get_obj();
    assert(metaDict["__class__"] == "TeamInfo");
    json_spirit::mObject dict = metaDict["__value__"].get_obj();

    std::unique_ptr<TeamInfo> team(new TeamInfo);
    team->name = dict["name"].get_str();
    team->members = toBotInfoVector(dict["members"].get_array());
    team->flag = toFlagInfo(dict["flag"].get_str());
    team->flagSpawnLocation = toVector2(dict["flagSpawnLocation"].get_array());
    team->flagScoreLocation = toVector2(dict["flagScoreLocation"].get_array());
    team->botSpawnArea = toVector2Pair(dict["botSpawnArea"].get_array());
    return team;
}

template <>
inline std::unique_ptr<BotInfo> fromJSON<>(const json_spirit::mValue& value)
{
    assert(value.type() == json_spirit::obj_type);
    json_spirit::mObject metaDict = value.get_obj();
    assert(metaDict["__class__"] == "BotInfo");
    json_spirit::mObject dict = metaDict["__value__"].get_obj();

    std::unique_ptr<BotInfo> bot(new BotInfo);
    bot->name = dict["name"].get_str();
    bot->team = toTeamInfo(dict["team"].get_str());
    bot->health = dict["health"].is_null() ? boost::optional<float>() : float(dict["health"].get_real());
    bot->state = dict["state"].is_null() ? boost::optional<BotInfo::State>() : BotInfo::State(dict["state"].get_int());
    bot->position = dict["position"].is_null() ? boost::optional<Vector2>() : toVector2(dict["position"].get_array());
    bot->facingDirection = dict["facingDirection"].is_null() ? boost::optional<Vector2>() : toVector2(dict["facingDirection"].get_array());
    bot->seenlast = dict["seenlast"].is_null() ? boost::optional<float>() : float(dict["seenlast"].get_real());
    bot->flag = dict["flag"].is_null() ? nullptr : toFlagInfo(dict["flag"].get_str());
    bot->visibleEnemies = toBotInfoVector(dict["visibleEnemies"].get_array());
    bot->seenBy = toBotInfoVector(dict["seenBy"].get_array());
    return bot;
}

template <>
inline std::unique_ptr<FlagInfo> fromJSON<>(const json_spirit::mValue& value)
{
    assert(value.type() == json_spirit::obj_type);
    json_spirit::mObject metaDict = value.get_obj();
    assert(metaDict["__class__"] == "FlagInfo");
    json_spirit::mObject dict = metaDict["__value__"].get_obj();

    std::unique_ptr<FlagInfo> flag(new FlagInfo);
    flag->name = dict["name"].get_str();
    flag->team = toTeamInfo(dict["team"].get_str());
    flag->position = toVector2(dict["position"].get_array());
    flag->carrier = dict["carrier"].is_null() ? nullptr : toBotInfo(dict["carrier"].get_str());
    flag->respawnTimer = float(dict["respawnTimer"].get_real());
    return flag;
}

template <>
inline std::unique_ptr<MatchInfo> fromJSON<>(const json_spirit::mValue& value)
{
    assert(value.type() == json_spirit::obj_type);
    json_spirit::mObject metaDict = value.get_obj();
    assert(metaDict["__class__"] == "MatchInfo");
    json_spirit::mObject dict = metaDict["__value__"].get_obj();

    std::unique_ptr<MatchInfo> match(new MatchInfo);
    match->scores = toStringFloatMap(dict["scores"].get_obj());
    match->timeRemaining = float(dict["timeRemaining"].get_real());
    match->timeToNextRespawn = float(dict["timeToNextRespawn"].get_real());
    match->timePassed = float(dict["timePassed"].get_real());
    match->combatEvents = toMatchCombatEventVector(dict["combatEvents"].get_array());
    return match;
}

template <>
inline std::unique_ptr<GameInfo> fromJSON<>(const json_spirit::mValue& value)
{
    assert(value.type() == json_spirit::obj_type);
    json_spirit::mObject metaDict = value.get_obj();
    assert(metaDict["__class__"] == "GameInfo");
    json_spirit::mObject dict = metaDict["__value__"].get_obj();

    std::unique_ptr<GameInfo> game(new GameInfo);
    game->match = fromJSON<MatchInfo>(dict["match"].get_obj());
    game->teams = toStringTeamInfoMap(dict["teams"].get_obj());
    game->team = toTeamInfo(dict["team"].get_str());
    game->enemyTeam = toTeamInfo(dict["enemyTeam"].get_str());
    game->bots = toStringBotInfoMap(dict["bots"].get_obj());
    game->flags = toStringFlagInfoMap(dict["flags"].get_obj());
    return game;
}

template <>
inline std::unique_ptr<MatchCombatEvent> fromJSON<>(const json_spirit::mValue& value);

inline json_spirit::mArray toArray(const Vector2& v)
{
    json_spirit::mArray arr;
    arr.push_back(v.x);
    arr.push_back(v.y);
    return arr;
}

inline json_spirit::mArray toArray(const vector<Vector2>& v)
{
    json_spirit::mArray arr;
    for (auto i = v.begin(), end = v.end(); i != end; ++i)
    {
        arr.push_back(toArray(*i));
    }
    return arr;
}

inline json_spirit::mArray toArray(const DefendCommand::FacingDirectionVector::value_type& pair)
{
    json_spirit::mArray arr;
    arr.push_back(toArray(pair.first));
    arr.push_back(pair.second);
    return arr;
}

inline json_spirit::mArray toArray(const DefendCommand::FacingDirectionVector& v)
{
    json_spirit::mArray arr;
    for (auto i = v.begin(), end = v.end(); i != end; ++i)
    {
        arr.push_back(toArray(*i));
    }
    return arr;
}


inline std::string toJSON(const ConnectServer& object)
{
    mObject jsonObj;
    jsonObj["protocolVersion"] = object.m_protocolVersion;

    mObject jsonMetaObj;
    jsonMetaObj["__class__"] = "ConnectServer";
    jsonMetaObj["__value__"] = jsonObj;

    json_spirit::mValue value(jsonMetaObj);                                                            
    string text = json_spirit::write_string(value); // , json_spirit::pretty_print);
    return text;
}

inline std::string toJSON(const ConnectClient& object)
{
    mObject jsonObj;
    jsonObj["commanderName"] = object.m_commanderName;
    jsonObj["language"] = object.m_language;

    mObject jsonMetaObj;
    jsonMetaObj["__class__"] = "ConnectClient";
    jsonMetaObj["__value__"] = jsonObj;

    json_spirit::mValue value(jsonMetaObj);
    string text = json_spirit::write_string(value); // , json_spirit::pretty_print);
    return text;
}

inline std::string toJSON(const DefendCommand& object)
{
    mObject jsonObj;
    jsonObj["bot"] = object.m_botId;
    jsonObj["facingDirections"] = object.m_facingDirections ? toArray(*object.m_facingDirections) : mValue::null;
    jsonObj["description"] = object.m_description;

    mObject jsonMetaObj;
    jsonMetaObj["__class__"] = "Defend";
    jsonMetaObj["__value__"] = jsonObj;

    json_spirit::mValue value(jsonMetaObj);
    string text = json_spirit::write_string(value); // , json_spirit::pretty_print);
    return text;
}

inline std::string toJSON(const MoveCommand& object)
{
    mObject jsonObj;
    jsonObj["bot"] = object.m_botId;
    jsonObj["target"] = toArray(object.m_target);
    jsonObj["description"] = object.m_description;

    mObject jsonMetaObj;
    jsonMetaObj["__class__"] = "Move";
    jsonMetaObj["__value__"] = jsonObj;

    json_spirit::mValue value(jsonMetaObj);
    string text = json_spirit::write_string(value); // , json_spirit::pretty_print);
    return text;
}

inline std::string toJSON(const AttackCommand& object)
{
    mObject jsonObj;
    jsonObj["bot"] = object.m_botId;
    jsonObj["target"] = toArray(object.m_target);
    jsonObj["lookAt"] = object.m_lookAt ? toArray(*object.m_lookAt) : mValue::null;
    jsonObj["description"] = object.m_description;

    mObject jsonMetaObj;
    jsonMetaObj["__class__"] = "Attack";
    jsonMetaObj["__value__"] = jsonObj;

    json_spirit::mValue value(jsonMetaObj);
    string text = json_spirit::write_string(value); // , json_spirit::pretty_print);
    return text;
}

inline std::string toJSON(const ChargeCommand& object)
{
    mObject jsonObj;
    jsonObj["bot"] = object.m_botId;
    jsonObj["target"] = toArray(object.m_target);
    jsonObj["description"] = object.m_description;

    mObject jsonMetaObj;
    jsonMetaObj["__class__"] = "Charge";
    jsonMetaObj["__value__"] = jsonObj;

    json_spirit::mValue value(jsonMetaObj);
    string text = json_spirit::write_string(value); // , json_spirit::pretty_print);
    return text;
}

inline std::string toJSON(const Command& object)
{
    const DefendCommand* defend = dynamic_cast<const DefendCommand*>(&object);
    if (defend)
    {
        return toJSON(*defend);
    }
    const MoveCommand* move = dynamic_cast<const MoveCommand*>(&object);
    if (move)
    {
        return toJSON(*move);
    }
    const AttackCommand* attack = dynamic_cast<const AttackCommand*>(&object);
    if (attack)
    {
        return toJSON(*attack);
    }
    const ChargeCommand* charge = dynamic_cast<const ChargeCommand*>(&object);
    if (charge)
    {
        return toJSON(*charge);
    }
    return "";
}

inline Vector2 toVector2(const json_spirit::mArray& arr)
{
    assert(arr.size() == 2);
    return Vector2(float(arr[0].get_real()), float(arr[1].get_real()));
}

inline vector<string> toStringVector(const json_spirit::mArray& arr)
{
    vector<string> v;
    for (json_spirit::mArray::const_iterator iter = arr.begin(); iter != arr.end(); ++iter)
    {
        v.push_back(iter->get_str());
    }
    return v;
}

inline TeamInfo* toTeamInfo(const string& name)
{
    TeamInfo* flag = new TeamInfo();
    flag->name = name;
    return flag;
}

inline vector<TeamInfo*> toTeamInfoVector(const json_spirit::mArray& arr)
{
    vector<TeamInfo*> v;
    for (json_spirit::mArray::const_iterator iter = arr.begin(); iter != arr.end(); ++iter)
    {
        v.push_back(toTeamInfo(iter->get_str()));
    }
    return v;
}

inline FlagInfo* toFlagInfo(const string& name)
{
    FlagInfo* flag = new FlagInfo();
    flag->name = name;
    return flag;
}

inline vector<FlagInfo*> toFlagInfoVector(const json_spirit::mArray& arr)
{
    vector<FlagInfo*> v;
    for (json_spirit::mArray::const_iterator iter = arr.begin(); iter != arr.end(); ++iter)
    {
        v.push_back(toFlagInfo(iter->get_str()));
    }
    return v;
}

inline BotInfo* toBotInfo(const string& name)
{
    BotInfo* bot = new BotInfo();
    bot->name = name;
    return bot;
}

inline vector<BotInfo*> toBotInfoVector(const json_spirit::mArray& arr)
{
    vector<BotInfo*> v;
    for (json_spirit::mArray::const_iterator iter = arr.begin(); iter != arr.end(); ++iter)
    {
        v.push_back(toBotInfo(iter->get_str()));
    }
    return v;
}

inline MatchCombatEvent toMatchCombatEvent(const json_spirit::mValue& value)
{
    assert(value.type() == json_spirit::obj_type);
    json_spirit::mObject metaDict = value.get_obj();
    assert(metaDict["__class__"] == "MatchCombatEvent");
    json_spirit::mObject dict = metaDict["__value__"].get_obj();

    MatchCombatEvent event;
    event.type = MatchCombatEvent::Type(dict["type"].get_int());
    event.time = float(dict["time"].get_real());
    switch (event.type)
    {
    case MatchCombatEvent::TYPE_KILLED:
        event.killedEventData.instigator = toBotInfo(dict["instigator"].get_str());
        event.killedEventData.subject = toBotInfo(dict["subject"].get_str());
        break;
    case MatchCombatEvent::TYPE_FLAG_PICKEDUP:
    case MatchCombatEvent::TYPE_FLAG_DROPPED:
    case MatchCombatEvent::TYPE_FLAG_CAPTURED:
        // these events all have the same data format
        event.flagPickupEventData.instigator = toBotInfo(dict["instigator"].get_str());
        event.flagPickupEventData.subject = toFlagInfo(dict["subject"].get_str());
        break;
    case MatchCombatEvent::TYPE_FLAG_RESTORED:
        assert(dict["instigator"].is_null());
        event.flagRestoredEventData.subject = toFlagInfo(dict["subject"].get_str());
        break;
    case MatchCombatEvent::TYPE_RESPAWN:
        assert(dict["instigator"].is_null());
        event.respawnEventData.subject = toBotInfo(dict["subject"].get_str());
        break;
    case MatchCombatEvent::TYPE_NONE:
        assert(false);
    }

    return event;
}

inline vector<MatchCombatEvent> toMatchCombatEventVector(const json_spirit::mArray& arr)
{
    vector<MatchCombatEvent> v;
    for (json_spirit::mArray::const_iterator iter = arr.begin(); iter != arr.end(); ++iter)
    {
        v.push_back(toMatchCombatEvent(iter->get_obj()));
    }
    return v;
}

inline map<string, float> toStringFloatMap(const json_spirit::mObject& dict)
{
    map<string, float> m;
    for (json_spirit::mObject::const_iterator iter = dict.begin(); iter != dict.end(); ++iter)
    {
        m[iter->first] = (float)iter->second.get_real();
    }
    return m;
}

inline map<string, Vector2> toStringVector2Map(const json_spirit::mObject& dict)
{
    map<string, Vector2> m;
    for (json_spirit::mObject::const_iterator iter = dict.begin(); iter != dict.end(); ++iter)
    {
        m[iter->first] = toVector2(iter->second.get_array());
    }
    return m;
}

inline pair<Vector2, Vector2> toVector2Pair(const json_spirit::mArray& arr)
{
    assert(arr.size() == 2);
    Vector2 first = toVector2(arr[0].get_array());
    Vector2 second = toVector2(arr[1].get_array());
    return make_pair(first, second);
}

inline map<string, pair<Vector2, Vector2> > toStringVector2PairMap(const json_spirit::mObject& dict)
{
    map<string, pair<Vector2, Vector2> > m;
    for (json_spirit::mObject::const_iterator iter = dict.begin(); iter != dict.end(); ++iter)
    {
        m[iter->first] = toVector2Pair(iter->second.get_array());
    }
    return m;
}

inline map<string, unique_ptr<TeamInfo> > toStringTeamInfoMap(const json_spirit::mObject& dict)
{
    map<string, unique_ptr<TeamInfo> > m;
    for (json_spirit::mObject::const_iterator iter = dict.begin(); iter != dict.end(); ++iter)
    {
        m[iter->first] = fromJSON<TeamInfo>(iter->second.get_obj());
    }
    return m;
}

inline map<string, unique_ptr<BotInfo> > toStringBotInfoMap(const json_spirit::mObject& dict)
{
    map<string, unique_ptr<BotInfo> > m;
    for (json_spirit::mObject::const_iterator iter = dict.begin(); iter != dict.end(); ++iter)
    {
        m[iter->first] = fromJSON<BotInfo>(iter->second.get_obj());
    }
    return m;
}

inline map<string, unique_ptr<FlagInfo> > toStringFlagInfoMap(const json_spirit::mObject& dict)
{
    map<string, unique_ptr<FlagInfo> > m;
    for (json_spirit::mObject::const_iterator iter = dict.begin(); iter != dict.end(); ++iter)
    {
        m[iter->first] = fromJSON<FlagInfo>(iter->second.get_obj());
    }
    return m;
}


inline unique_ptr<float[]> toFloatArray(const json_spirit::mArray& arr, int length)
{
    float* p = new float[length];

    for(int i=0; i<length; ++i)
    {
        p[i] = (float)arr[i].get_real();
    }

    return unique_ptr<float[]>(p);
}

inline unique_ptr<float[]> toFloatArray(const json_spirit::mArray& arr, int width, int height) 
{
    float* p = new float[width * height];

    for(int x=0; x<width; ++x)
    {
        const json_spirit::mArray& row = arr[x].get_array();
        for(int y=0; y<height; ++y)
            p[x+y*width] = (float)row[y].get_real();
    }

    //std::transform(arr.begin(), arr.end(), p, [] (const mValue& x) -> float { return (float)(x.get_real()); } );
    return unique_ptr<float[]>(p);
}


inline void replaceTemporaryTeamInfo(TeamInfo*& tempTeamInfo, GameInfo& gameInfo)
{
    if (tempTeamInfo)
    {
        TeamInfo* teamInfo = gameInfo.teams[tempTeamInfo->name].get();
        delete tempTeamInfo;
        tempTeamInfo = teamInfo;
    }
}

inline void replaceTemporaryFlagInfo(FlagInfo*& tempFlagInfo, GameInfo& gameInfo)
{
    if (tempFlagInfo)
    {
        FlagInfo* flagInfo = gameInfo.flags[tempFlagInfo->name].get();
        delete tempFlagInfo;
        tempFlagInfo = flagInfo;
    }
}

inline void replaceTemporaryBotInfo(BotInfo*& tempBotInfo, GameInfo& gameInfo)
{
    if (tempBotInfo)
    {
        BotInfo* botInfo = gameInfo.bots[tempBotInfo->name].get();
        delete tempBotInfo;
        tempBotInfo = botInfo;
    }
}

inline void fixupReferences(TeamInfo& teamInfo, GameInfo& gameInfo)
{
    for (auto botIter = teamInfo.members.begin(); botIter != teamInfo.members.end(); ++botIter)
    {
        replaceTemporaryBotInfo(*botIter, gameInfo);
    }
    replaceTemporaryFlagInfo(teamInfo.flag, gameInfo);
}

inline void fixupReferences(FlagInfo& flagInfo, GameInfo& gameInfo)
{
    replaceTemporaryTeamInfo(flagInfo.team, gameInfo);
}

inline void fixupReferences(BotInfo& botInfo, GameInfo& gameInfo)
{
    replaceTemporaryTeamInfo(botInfo.team, gameInfo);
    replaceTemporaryFlagInfo(botInfo.flag, gameInfo);
    for (auto enemyIter = botInfo.visibleEnemies.begin(); enemyIter != botInfo.visibleEnemies.end(); ++enemyIter)
    {
        replaceTemporaryBotInfo(*enemyIter, gameInfo);
    }
    for (auto enemyIter = botInfo.seenBy.begin(); enemyIter != botInfo.seenBy.end(); ++enemyIter)
    {
        replaceTemporaryBotInfo(*enemyIter, gameInfo);
    }
}

inline void fixupReferences(MatchInfo& matchInfo, GameInfo& gameInfo)
{
    for (auto eventIter = matchInfo.combatEvents.begin(); eventIter != matchInfo.combatEvents.end(); ++eventIter)
    {
        MatchCombatEvent& event = *eventIter;
        switch (event.type)
        {
        case MatchCombatEvent::TYPE_KILLED:
            replaceTemporaryBotInfo(event.killedEventData.instigator, gameInfo);
            replaceTemporaryBotInfo(event.killedEventData.subject, gameInfo);
            break;
        case MatchCombatEvent::TYPE_FLAG_PICKEDUP:
        case MatchCombatEvent::TYPE_FLAG_DROPPED:
        case MatchCombatEvent::TYPE_FLAG_CAPTURED:
            // these events all have the same data format
            replaceTemporaryBotInfo(event.flagPickupEventData.instigator, gameInfo);
            replaceTemporaryFlagInfo(event.flagPickupEventData.subject, gameInfo);
            break;
        case MatchCombatEvent::TYPE_FLAG_RESTORED:
            replaceTemporaryFlagInfo(event.flagRestoredEventData.subject, gameInfo);
            break;
        case MatchCombatEvent::TYPE_RESPAWN:
            replaceTemporaryBotInfo(event.respawnEventData.subject, gameInfo);
            break;
        case MatchCombatEvent::TYPE_NONE:
            assert(false);
        }
    }
}

inline void fixupGameInfoReferences(GameInfo& gameInfo)
{
    replaceTemporaryTeamInfo(gameInfo.team, gameInfo);
    replaceTemporaryTeamInfo(gameInfo.enemyTeam, gameInfo);
    for (auto teamIter = gameInfo.teams.begin(); teamIter != gameInfo.teams.end(); ++teamIter)
    {
        fixupReferences(*teamIter->second, gameInfo);
    }
    for (auto botIter = gameInfo.flags.begin(); botIter != gameInfo.flags.end(); ++botIter)
    {
        fixupReferences(*botIter->second, gameInfo);
    }
    for (auto botIter = gameInfo.bots.begin(); botIter != gameInfo.bots.end(); ++botIter)
    {
        fixupReferences(*botIter->second, gameInfo);
    }
    fixupReferences(*gameInfo.match, gameInfo);
}

inline void mergeFlagInfo(GameInfo& gameInfo, FlagInfo& newFlagInfo)
{
    fixupReferences(newFlagInfo, gameInfo);
    FlagInfo& flagInfo = *gameInfo.flags[newFlagInfo.name];
    flagInfo = newFlagInfo;
}

inline void mergeBotInfo(GameInfo& gameInfo, BotInfo& newBotInfo)
{
    fixupReferences(newBotInfo, gameInfo);
    BotInfo& botInfo = *gameInfo.bots[newBotInfo.name];
    botInfo = newBotInfo;
}

inline void mergeMatchInfo(GameInfo& gameInfo, MatchInfo& newMatchInfo)
{
    fixupReferences(newMatchInfo, gameInfo);
    MatchInfo& matchInfo = *gameInfo.match;
    matchInfo.scores = newMatchInfo.scores;
    matchInfo.timeRemaining = newMatchInfo.timeRemaining;
    matchInfo.timeToNextRespawn = newMatchInfo.timeToNextRespawn;
    matchInfo.timePassed = newMatchInfo.timePassed;
    // append the new combat events onto the end of the existing ones
    matchInfo.combatEvents.insert(matchInfo.combatEvents.end(), newMatchInfo.combatEvents.begin(), newMatchInfo.combatEvents.end());
}


inline void mergeGameInfo(GameInfo& gameInfo, GameInfo& newGameInfo)
{
    for (auto flagIter = newGameInfo.flags.begin(); flagIter != newGameInfo.flags.end(); ++flagIter)
    {
        mergeFlagInfo(gameInfo, *flagIter->second);
    }

    for (auto botIter = newGameInfo.bots.begin(); botIter != newGameInfo.bots.end(); ++botIter)
    {
        mergeBotInfo(gameInfo, *botIter->second);
    }

    mergeMatchInfo(gameInfo, *newGameInfo.match);
}

#endif // JSON_H
