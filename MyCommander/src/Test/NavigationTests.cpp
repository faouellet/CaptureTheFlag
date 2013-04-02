#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include "NavigationFixture.h"

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* Navigation Tests
* The goal of these tests is to verify the correctness and the scalability
* of the path planning algorithm implemented by the Navigator
*/
BOOST_FIXTURE_TEST_SUITE( NavigationTestSuite, NavigationFixture )

// The correctness test aim to emulate the path and the clusters formed in the following example:
// http://jagedev.blogspot.ca/2011/08/hierarchical-pathfinding-explained_28.html

BOOST_AUTO_TEST_CASE( PathCorrectnessTest )
{
	std::vector<Navigator::Node> l_ExpectedAbstractPath;
	l_ExpectedAbstractPath.push_back(Navigator::Node(1, 0, Vector2(0.f, 6.f)));
	l_ExpectedAbstractPath.push_back(Navigator::Node(1, 0, Vector2(3.f, 6.f)));
	l_ExpectedAbstractPath.push_back(Navigator::Node(1, 0, Vector2(4.f, 6.f)));
	l_ExpectedAbstractPath.push_back(Navigator::Node(1, 0, Vector2(7.f, 4.f)));
	l_ExpectedAbstractPath.push_back(Navigator::Node(1, 0, Vector2(7.f, 3.f)));
	l_ExpectedAbstractPath.push_back(Navigator::Node(1, 0, Vector2(4.f, 1.f)));
	l_ExpectedAbstractPath.push_back(Navigator::Node(1, 0, Vector2(3.f, 1.f)));
	l_ExpectedAbstractPath.push_back(Navigator::Node(1, 0, Vector2(1.f, 1.f)));

	std::vector<Vector2> l_ExpectedConcretePath;
	l_ExpectedConcretePath.push_back(Vector2(0.f, 6.f));
	l_ExpectedConcretePath.push_back(Vector2(1.f, 5.f));
	l_ExpectedConcretePath.push_back(Vector2(2.f, 4.f));
	l_ExpectedConcretePath.push_back(Vector2(3.f, 5.f));
	l_ExpectedConcretePath.push_back(Vector2(3.f, 6.f));
	l_ExpectedConcretePath.push_back(Vector2(4.f, 6.f));
	l_ExpectedConcretePath.push_back(Vector2(5.f, 6.f));
	l_ExpectedConcretePath.push_back(Vector2(6.f, 5.f));
	l_ExpectedConcretePath.push_back(Vector2(7.f, 4.f));
	l_ExpectedConcretePath.push_back(Vector2(7.f, 3.f));
	l_ExpectedConcretePath.push_back(Vector2(6.f, 2.f));
	l_ExpectedConcretePath.push_back(Vector2(5.f, 1.f));
	l_ExpectedConcretePath.push_back(Vector2(4.f, 1.f));
	l_ExpectedConcretePath.push_back(Vector2(3.f, 1.f));
	l_ExpectedConcretePath.push_back(Vector2(2.f, 1.f));
	l_ExpectedConcretePath.push_back(Vector2(1.f, 1.f));

	m_Nav.Init(m_SmallLevel->blockHeights, m_SmallLevel->height, m_SmallLevel->width, 4);

	std::vector<Navigator::Node> l_AbstractPath(m_Nav.ComputeAbstractPath(m_StartPos, m_GoalPos));
	BOOST_REQUIRE(l_AbstractPath.size() == l_ExpectedAbstractPath.size());
	for(unsigned i = 0; i < l_AbstractPath.size(); ++i)
		BOOST_REQUIRE(l_AbstractPath[i] == l_ExpectedAbstractPath[i]);

	std::vector<Vector2> l_ConcretePath;
	for(unsigned i = 0; i < l_AbstractPath.size()-1; ++i)
	{
		vector<Vector2> l_Path(m_Nav.ComputeConcretePath(l_AbstractPath[i], l_AbstractPath[i+1]));
		l_ConcretePath.insert(l_ConcretePath.end(), l_Path.begin(), l_Path.end());
	}
	BOOST_REQUIRE(l_ConcretePath.size() == l_ExpectedConcretePath.size());
	for(unsigned i = 0; i < l_ConcretePath.size(); ++i)
		BOOST_REQUIRE(l_ConcretePath[i] == l_ExpectedConcretePath[i]);
}

BOOST_AUTO_TEST_CASE( InitPerformanceTest )
{
	std::vector<boost::chrono::duration<double, boost::milli>> l_Durations(100);
	boost::chrono::high_resolution_clock::time_point l_Start;
		
	for(int i = 0; i < 100; ++i)
	{
		l_Start = boost::chrono::high_resolution_clock::now();
		m_Nav.Init(m_MediumLevel->blockHeights, m_MediumLevel->height, m_MediumLevel->width);
		l_Durations[i] = boost::chrono::high_resolution_clock::now() - l_Start;
		m_Nav.Reset();
	}

	std::vector<double> l_Times(ToVectorOfDouble(l_Durations));
	BOOST_REQUIRE(ComputeMean(l_Times) < MAX_INIT_TIME);
}

BOOST_AUTO_TEST_CASE( SmallGraphPerformanceTest )
{
	BOOST_REQUIRE(TestPerformance(m_SmallLevel->blockHeights, m_SmallLevel->height, m_SmallLevel->width, 4));
}

BOOST_AUTO_TEST_CASE( NormalGraphPerformanceTest )
{
	BOOST_REQUIRE(TestPerformance(m_MediumLevel->blockHeights, m_MediumLevel->height, m_MediumLevel->width));
}

BOOST_AUTO_TEST_SUITE_END()
