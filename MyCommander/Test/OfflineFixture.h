#ifndef ONLINE_FIXTURE_H
#define ONLINE_FIXTURE_H

#include <functional>

#include "api\GameInfo.h"
#include "api\json.h"
#include "MyCommander.cpp"

#include "Utils.h"

/*
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

	MyCommander m_Cmd;

	OfflineFixture()
	{
		std::string l_GameInitStr = ReadAllFile("GameInit.json");
		std::string l_SmallGameTickStr = ReadAllFile("SmallGameTick.json");
		std::string l_MediumGameTickStr = ReadAllFile("MediumGameTick.json");
		std::string l_LargeGameTickStr = ReadAllFile("LargeGameTick.json");

		std::string l_SmallLevelStr = ReadAllFile("SmallLevel.json");
		std::string l_MediumLevelStr = ReadAllFile("MediumLevel.json");
		std::string l_LargeLevelStr = ReadAllFile("LargeLevel.json");

		json_spirit::read_string(l_GameInitStr, m_GameInitValue);
		json_spirit::read_string(l_SmallGameTickStr, m_SmallGameTickValue);
		json_spirit::read_string(l_MediumGameTickStr, m_MediumGameTickValue);
		json_spirit::read_string(l_LargeGameTickStr, m_LargeGameTickValue);

		json_spirit::read_string(l_SmallLevelStr, m_SmallLevelValue);
		json_spirit::read_string(l_MediumLevelStr, m_MediumLevelValue);
		json_spirit::read_string(l_LargeLevelStr, m_LargeLevelValue);
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
};

const double OfflineFixture::MAX_TIME_INIT = 5000.0;
const double OfflineFixture::MAX_TIME_TICK = 80.0;

#endif // ONLINE_FIXTURE_H
