/*

*/

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

// The correctness tests aim to reproduce the path and the clusters formed in the following example:
// http://jagedev.blogspot.ca/2011/08/hierarchical-pathfinding-explained_28.html

BOOST_AUTO_TEST_CASE( ClusterCorrectnessTest )
{
	BOOST_REQUIRE(true);
};

BOOST_AUTO_TEST_CASE( PathCorrectnessTest )
{
	std::vector<Vector2> l_CompletePath;
	l_CompletePath.push_back(m_StartPos);
	l_CompletePath.push_back(Vector2(1,6));
	l_CompletePath.push_back(Vector2(1,5));
	l_CompletePath.push_back(Vector2(1,4));
	l_CompletePath.push_back(Vector2(2,4));
	l_CompletePath.push_back(Vector2(3,4));
	l_CompletePath.push_back(Vector2(4,4));
	l_CompletePath.push_back(Vector2(4,5));
	l_CompletePath.push_back(Vector2(5,5));
	l_CompletePath.push_back(Vector2(6,5));
	l_CompletePath.push_back(Vector2(6,4));
	l_CompletePath.push_back(Vector2(6,3));
	l_CompletePath.push_back(Vector2(6,2));
	l_CompletePath.push_back(Vector2(6,1));
	l_CompletePath.push_back(Vector2(5,1));
	l_CompletePath.push_back(Vector2(4,1));
	l_CompletePath.push_back(Vector2(3,1));
	l_CompletePath.push_back(Vector2(2,1));
	l_CompletePath.push_back(m_GoalPos);
	
	m_Nav.Init(m_SmallLevel->blockHeights, m_SmallLevel->height, m_SmallLevel->width, 4);

	for(unsigned i = 0; i < l_CompletePath.size() - 1; ++i)
	{
		BOOST_REQUIRE(m_Nav.GetBestDirection(l_CompletePath[i], m_GoalPos) == l_CompletePath[i+1]);
	}
}

BOOST_AUTO_TEST_CASE( SmallGraphPerformanceTest )
{
	m_Nav.Init(m_SmallLevel->blockHeights, m_SmallLevel->height, m_SmallLevel->width, 4);
	BOOST_REQUIRE(TestPerformance());
}

BOOST_AUTO_TEST_CASE( MediumGraphPerformanceTest )
{
	m_Nav.Init(m_MediumLevel->blockHeights, m_MediumLevel->height, m_MediumLevel->width);
	BOOST_REQUIRE(TestPerformance());
}

BOOST_AUTO_TEST_CASE( LargeGraphPerformanceTest )
{
	m_Nav.Init(m_LargeLevel->blockHeights, m_LargeLevel->height, m_LargeLevel->width);
	BOOST_REQUIRE(TestPerformance());
}

BOOST_AUTO_TEST_SUITE_END()