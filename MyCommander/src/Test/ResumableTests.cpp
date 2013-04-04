#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include "Resumable.h"

#include <vector>

BOOST_AUTO_TEST_SUITE( ResumableTestSuite )

BOOST_AUTO_TEST_CASE( ResumableForEachTest )
{
	std::vector<int> l_TestVector(100,1);

	auto l_Begin = l_TestVector.begin();
	auto l_End = l_TestVector.end();

	int l_Sum = 0;
	int l_Cpt;

	for(int i = 1; i <= 10; ++i)
	{
		l_Cpt = 0;
		ResumableForEach(l_Begin, l_End, [&l_Sum, &l_Begin](){ l_Sum += *l_Begin; }, [&l_Cpt](){ return l_Cpt++ < 10; });
		BOOST_REQUIRE(--l_Cpt == 10);
		BOOST_REQUIRE(l_Sum == l_Cpt * i);
	}
};

BOOST_AUTO_TEST_CASE( ResumableEmbeddedLoopTest )
{
	std::vector<int> l_TestVector(10);

	auto l_Begin1 = l_TestVector.begin();
	auto l_Begin2 = l_TestVector.begin() + 1;
	auto l_End = l_TestVector.end();

	int l_NbInnerIters;

	for(int i = 1; i < 11; ++i)
	{
		l_NbInnerIters = 0;
		ResumableEmbeddedLoop(l_Begin1, l_Begin2, l_End, [&l_NbInnerIters](){ ++l_NbInnerIters; }, [&l_NbInnerIters, &i](){ return l_NbInnerIters < 10 - i; });
		BOOST_REQUIRE(l_NbInnerIters == 10-i);
		BOOST_REQUIRE(l_Begin2 == l_End);
	}
};

BOOST_AUTO_TEST_SUITE_END()
