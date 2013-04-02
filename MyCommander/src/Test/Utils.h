#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <fstream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

#include <boost/chrono/chrono.hpp>

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
* 
* Utils
*/

inline double ComputeMean(std::vector<double>& in_Durations)
{
	// We'll discard the 5 slowest and 5 fastest to have a better estimate of our mean
	std::sort(in_Durations.begin(), in_Durations.end());
	double l_MeanElapsedTime = std::accumulate(in_Durations.begin()+5, in_Durations.end()-5, 0.0, 
		[](double io_Sum, double in_Duration) -> double
	{
		return io_Sum + in_Duration;
	});

	return l_MeanElapsedTime /= (in_Durations.size() - 10);
}

inline std::vector<double> ToVectorOfDouble(const std::vector<boost::chrono::duration<double, boost::milli>> & l_Durations)
{
	std::vector<double> l_Times;
	std::transform(l_Durations.begin(), l_Durations.end(), std::back_inserter(l_Times), 
		[](const boost::chrono::duration<double, boost::milli>& in_Duration)
	{
		return in_Duration.count();
	});

	return l_Times;
}

inline std::string ReadAllFile(const std::string & in_FileName)
{
	std::string l_FileContent;
	std::ifstream l_FileStream(in_FileName, std::ios::in | std::ios::binary);

	if(l_FileStream.is_open())
	{
		l_FileStream.seekg(0, std::ios::end);
		unsigned l_Size = static_cast<unsigned>(l_FileStream.tellg());
		l_FileContent.resize(l_Size);
		l_FileStream.seekg(0, std::ios::beg);
		l_FileStream.read(&l_FileContent[0], l_FileContent.size());
		l_FileStream.close();
	}
	return l_FileContent;
}

#endif // UTILS_H
