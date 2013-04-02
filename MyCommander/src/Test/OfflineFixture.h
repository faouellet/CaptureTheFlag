#ifndef ONLINE_FIXTURE_H
#define ONLINE_FIXTURE_H

#include <functional>

#include "api\GameInfo.h"
#include "api\json.h"
#include "MyCommander.cpp"

#include "Utils.h"

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* OfflineFixture
* Provides variables and functions needed to test the commander in an
* offline setting.
*/
struct OfflineFixture
{
	static const double MAX_TIME_INIT;
	static const double MAX_TIME_TICK;

	json_spirit::mValue m_GameInitValue;
	json_spirit::mValue m_SmallGameTickValue;
	json_spirit::mValue m_MediumGameTickValue;
	json_spirit::mValue m_LargeGameTickValue;

	json_spirit::mValue m_SmallLevelValue;
	json_spirit::mValue m_MediumLevelValue;
	json_spirit::mValue m_LargeLevelValue;

	std::unique_ptr<LevelInfo> m_SmallLevel;
	std::unique_ptr<LevelInfo> m_MediumLevel;

	MyCommander m_Cmd;

	OfflineFixture()
	{
		std::string l_GameInitStr = ReadAllFile("GameInit.json");
		std::string l_SmallGameTickStr = ReadAllFile("SmallGameTick.json");
		std::string l_MediumGameTickStr = ReadAllFile("MediumGameTick.json");

		std::string l_SmallLevelStr = ReadAllFile("SmallLevel.json");
		std::string l_MediumLevelStr = ReadAllFile("MediumLevel.json");

		json_spirit::read_string(l_GameInitStr, m_GameInitValue);
		json_spirit::read_string(l_SmallGameTickStr, m_SmallGameTickValue);
		json_spirit::read_string(l_MediumGameTickStr, m_MediumGameTickValue);

		json_spirit::read_string(l_SmallLevelStr, m_SmallLevelValue);
		json_spirit::read_string(l_MediumLevelStr, m_MediumLevelValue);

		m_SmallLevel = fromJSON<LevelInfo>(m_SmallLevelValue);
		m_MediumLevel = fromJSON<LevelInfo>(m_MediumLevelValue);
	}

	bool TestInitPerformance()
	{
		std::vector<boost::chrono::duration<double, boost::milli>> l_Durations(100);
		boost::chrono::high_resolution_clock::time_point l_Start;
		
		for(int i = 0; i < 100; ++i)
		{
			l_Start = boost::chrono::high_resolution_clock::now();
			m_Cmd.initialize(); 
			l_Durations[i] = boost::chrono::high_resolution_clock::now() - l_Start;
			m_Cmd.Reset();
		}

		std::vector<double> l_Times = ToVectorOfDouble(l_Durations);

		return ComputeMean(l_Times) < MAX_TIME_INIT;
	}

	bool TestTickPerformance()
	{
		std::vector<boost::chrono::duration<double, boost::milli>> l_Durations(100);
		boost::chrono::high_resolution_clock::time_point l_Start;
		
		for(int i = 0; i < 100; ++i)
		{
			l_Start = boost::chrono::high_resolution_clock::now();
			m_Cmd.tick();
			l_Durations[i] = boost::chrono::high_resolution_clock::now() - l_Start;
		}

		std::vector<double> l_Times = ToVectorOfDouble(l_Durations);

		return ComputeMean(l_Times) < MAX_TIME_TICK;
	}

	std::unique_ptr<GameInfo> InitGameInfo(const json_spirit::mValue & in_GameValue, const std::unique_ptr<LevelInfo> & in_Level) const
	{
		std::unique_ptr<GameInfo> l_GameInfo = fromJSON<GameInfo>(in_GameValue);

		for(auto l_It=l_GameInfo->teams["Blue"]->members.begin(); l_It != l_GameInfo->teams["Blue"]->members.end(); ++l_It)
		{
			BotInfo* l_Bot = *l_It;

			if(!l_Bot)
				continue;

			l_GameInfo->bots_available.push_back(l_Bot);
		}

		l_GameInfo->team->flagSpawnLocation = in_Level->flagSpawnLocations[l_GameInfo->team->name];
		for(auto l_BotIt = l_GameInfo->bots_available.begin(); l_BotIt != l_GameInfo->bots_available.end(); ++l_BotIt)
		{
			(*l_BotIt)->position = l_GameInfo->team->botSpawnArea.first;
			(*l_BotIt)->facingDirection = l_GameInfo->team->botSpawnArea.first;
		}
		return l_GameInfo;
	}
};

const double OfflineFixture::MAX_TIME_INIT = 5000.0;
const double OfflineFixture::MAX_TIME_TICK = 80.0;

#endif // ONLINE_FIXTURE_H
