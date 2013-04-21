#ifndef NAVIGATION_FIXTURE_H
#define NAVIGATION_FIXTURE_H

#include <memory>

#include "Navigator.h"
#include "Navigator.cpp"

#include "api\GameInfo.h"
#include "api\json.h"

#include "Utils.h"

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* NavigationFixture
* Provides variables and functions needed to test the navigation system
*/
struct NavigationFixture
{
	static const double MAX_INIT_TIME;
	static const double MAX_SEARCH_TIME;

	Navigator m_Nav;

	std::unique_ptr<LevelInfo> m_SmallLevel;
	std::unique_ptr<LevelInfo> m_MediumLevel;

	Vector2 m_StartPos;
	Vector2 m_GoalPos;

	NavigationFixture() : m_StartPos(0,6), m_GoalPos(1,1)
	{
		std::string l_SmallLevelStr = ReadAllFile("SmallLevel.json");
		std::string l_MediumLevelStr = ReadAllFile("MediumLevel.json");
		
		json_spirit::mValue l_SmallLevelValue;
		json_spirit::mValue l_MediumLevelValue;
		json_spirit::mValue l_LargeLevelValue;

		json_spirit::read_string(l_SmallLevelStr, l_SmallLevelValue);
		json_spirit::read_string(l_MediumLevelStr, l_MediumLevelValue);

		m_SmallLevel = fromJSON<LevelInfo>(l_SmallLevelValue);
		m_MediumLevel = fromJSON<LevelInfo>(l_MediumLevelValue);
	}

	bool TestPerformance(const std::unique_ptr<float[]> & in_Level, const int in_Length, const int in_Width, bool in_SmallTest,
		const int in_MaxEntranceWidth = 3)
	{
		std::vector<boost::chrono::duration<double, boost::milli>> l_AbstractDurations(100);
		std::vector<boost::chrono::duration<double, boost::milli>> l_ConcreteDurations;
		boost::chrono::high_resolution_clock::time_point l_Start;
		
		for(int i = 0; i < 100; ++i)
		{
			m_Nav.Init(in_Level, in_Length, in_Width, in_MaxEntranceWidth);
			l_Start = boost::chrono::high_resolution_clock::now();
			std::vector<std::shared_ptr<Navigator::Node>> l_AbstractPath(
				m_Nav.ComputeAbstractPath(in_SmallTest ? m_StartPos : Vector2(2.f,3.f), 
				in_SmallTest ? m_GoalPos : Vector2(70.f,48.f)));
			l_AbstractDurations[i] = boost::chrono::high_resolution_clock::now() - l_Start;

			std::vector<Vector2> l_ConcretePath;
			for(unsigned j = 0; j < l_AbstractPath.size()-1; ++j)
			{
				l_Start = boost::chrono::high_resolution_clock::now();
				vector<Vector2> l_Path(m_Nav.ComputeConcretePath(std::move(l_AbstractPath[j]), std::move(l_AbstractPath[j+1])));
				l_ConcreteDurations.push_back(boost::chrono::high_resolution_clock::now() - l_Start);
			}
			m_Nav.Reset();
		}

		std::vector<double> l_AbstractTimes(ToVectorOfDouble(l_AbstractDurations));
		std::vector<double> l_ConcreteTimes(ToVectorOfDouble(l_ConcreteDurations));

#ifdef _LOG_PERF
		std::ofstream l_ConcreteFileStream("Concrete Path Perf.txt", std::ios::out | std::ios::binary);
		if(l_ConcreteFileStream.is_open())
		{
			for(unsigned i = 0; i < l_AbstractTimes.size(); ++i)
			{
				l_ConcreteFileStream << "Pathfinding:" << i << " Duration: " << l_AbstractTimes[i] << std::endl;
			}
		}

		std::ofstream l_AbstractFileStream("Abstract Path Perf.txt", std::ios::out | std::ios::binary);
		if(l_AbstractFileStream.is_open())
		{
			for(unsigned i = 0; i < l_ConcreteTimes.size(); ++i)
			{
				l_AbstractFileStream << "Pathfinding:" << i << " Duration: " << l_ConcreteTimes[i] << std::endl;
			}
		}
#endif

		return ComputeMean(l_AbstractTimes) < MAX_SEARCH_TIME && ComputeMean(l_ConcreteTimes) < MAX_SEARCH_TIME;
	}
};

const double NavigationFixture::MAX_SEARCH_TIME = 80.0;
const double NavigationFixture::MAX_INIT_TIME = 7500.0;

#endif // NAVIGATION_FIXTURE_H
