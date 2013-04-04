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

class TrivialHeuristic : public IHeuristic
{
public:
	double operator()(const Navigator::Node & in_Start, const Navigator::Node & in_Goal) const
	{
		return 0.0;
	}
};

class EuclideanDistance : public IHeuristic
{
public:
	double operator()(const Navigator::Node & in_Start, const Navigator::Node & in_Goal) const
	{
		return sqrt(pow(in_Start.Position.x - in_Goal.Position.x, 2) + pow(in_Start.Position.y - in_Goal.Position.y, 2));
	}
};

class ManhattanDistance : public IHeuristic
{
public:
	double operator()(const Navigator::Node & in_Start, const Navigator::Node & in_Goal) const
	{
		return abs(in_Start.Position.x - in_Goal.Position.x) + abs(in_Start.Position.y - in_Goal.Position.y);
	}
};

class SeekHeuristic : public IHeuristic
{
private:
	std::vector<Vector2> m_EnemyLocations;

public:
	SeekHeuristic(const std::vector<Vector2> & in_EnemyLocations = std::vector<Vector2>()) : 
		m_EnemyLocations(in_EnemyLocations) { }

	double operator()(const Navigator::Node & in_Start, const Navigator::Node & in_Goal)
	{
		return 0.0;
	}
};

class AvoidHeuristic : public IHeuristic
{
private:
	std::vector<Vector2> m_EnemyLocations;

public:
	AvoidHeuristic(const std::vector<Vector2> & in_EnemyLocations = std::vector<Vector2>()) :
		m_EnemyLocations(in_EnemyLocations) { }

	double operator()(const Navigator::Node & in_Start, const Navigator::Node & in_Goal)
	{
		return 0.0;
	}
};

#endif // IHEURISTIC_H
