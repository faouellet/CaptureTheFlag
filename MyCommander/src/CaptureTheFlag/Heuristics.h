#ifndef IHEURISTIC_H
#define IHEURISTIC_H

#include "Navigator.h"

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*/

class IHeuristic
{
public:
	virtual double operator()(const Navigator::Node & in_Start, const Navigator::Node & in_Goal) const = 0;
};

class EuclideanDistance : public IHeuristic
{
public:
	double operator()(const Navigator::Node & in_Start, const Navigator::Node & in_Goal) const
	{
		return sqrt(pow(in_Start.Position.x - in_Goal.Position.x, 2) + pow(in_Start.Position.y - in_Goal.Position.y, 2));
	}
};

class TrivialHeuristic : public IHeuristic
{
public:
	double operator()(const Navigator::Node & in_Start, const Navigator::Node & in_Goal) const
	{
		return 0.0;
	}
};

#endif // IHEURISTIC_H
