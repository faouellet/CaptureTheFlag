#ifndef NAVIGATION_FIXTURE_H
#define NAVIGATION_FIXTURE_H

#include <memory>

#include "Navigator.h"
#include "Navigator.cpp"

#include "api\GameInfo.h"
#include "api\json.h"

#include "Utils.h"

/*
* NavigationFixture
* Provides variables and functions needed to test the navigation system
*/
struct NavigationFixture
{
	static const double MAX_SEARCH_TIME;

	Navigator m_Nav;
	std::unique_ptr<LevelInfo> m_SmallLevel;
	std::unique_ptr<LevelInfo> m_MediumLevel;
	std::unique_ptr<LevelInfo> m_LargeLevel;

	Vector2 m_StartPos;
	Vector2 m_GoalPos;

	NavigationFixture() : m_StartPos(0,6), m_GoalPos(1,1)
	{
		std::string l_SmallLevelStr = ReadAllFile("SmallLevel.json");
		std::string l_MediumLevelStr = ReadAllFile("MediumLevel.json");
		std::string l_LargeLevelStr = ReadAllFile("LargeLevel.json");
		
		json_spirit::mValue l_SmallLevelValue;
		json_spirit::mValue l_MediumLevelValue;
		json_spirit::mValue l_LargeLevelValue;

		json_spirit::read_string(l_SmallLevelStr, l_SmallLevelValue);
		json_spirit::read_string(l_MediumLevelStr, l_MediumLevelValue);
		json_spirit::read_string(l_LargeLevelStr, l_LargeLevelValue);

		m_SmallLevel = fromJSON<LevelInfo>(l_SmallLevelValue);
		m_MediumLevel = fromJSON<LevelInfo>(l_MediumLevelValue);
		m_LargeLevel = fromJSON<LevelInfo>(l_LargeLevelValue);
	}

	bool TestPerformance()
	{
		std::vector<boost::chrono::duration<double, boost::milli>> l_Durations(100);
		boost::chrono::high_resolution_clock::time_point l_Start;
		
		for(int i = 0; i < 100; ++i)
		{
			l_Start = boost::chrono::high_resolution_clock::now();
			m_Nav.GetBestDirection(m_StartPos, m_GoalPos);
			l_Durations[i] = boost::chrono::high_resolution_clock::now() - l_Start;
		}

		std::vector<double> l_Times = ToVectorOfDouble(l_Durations);

		return ComputeMean(l_Times) < MAX_SEARCH_TIME;
	}
};

// TODO : Need something a bit more specific?
const double NavigationFixture::MAX_SEARCH_TIME = 80.0;

#endif // NAVIGATION_FIXTURE_H
