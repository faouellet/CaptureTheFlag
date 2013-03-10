#include "Navigator.h"

#include <algorithm>

// TODO : Split in two functions
bool Navigator::Init(std::unique_ptr<float[]> & in_GameGrid, const int in_Width)
{
	bool l_Ok = false;
	if(!in_GameGrid)
		return l_Ok;

	std::vector<Node> l_Nodes;
	int l_X = 0;
	int l_Y = 0;
	int l_NbUntreatedElems = std::distance(in_GameGrid.get(), in_GameGrid.get() + sizeof(in_GameGrid.get()));
	unsigned int i = l_NbUntreatedElems;

	while(i > 0)
	{
		l_Nodes.push_back(Vector2(static_cast<float>(l_X), static_cast<float>(l_Y * in_Width)));
		--i;
		if(++l_X > in_Width)
		{
			l_X = 0;
			++l_Y;
		}
	}

	l_X = 0;
	l_Y = 0;

	for(i = 0; i < l_Nodes.size(); ++i)
	{
		// Don't link with an elevated cell
		if(!in_GameGrid[i])
			continue;

		unsigned int l_EastIndex = (l_X + 1) + (l_Y) * in_Width;
		unsigned int l_SouthEastIndex = (l_X + 1) + (l_Y + 1) * in_Width;
		unsigned int l_SouthIndex = (l_X) + (l_Y + 1) * in_Width;
		unsigned int l_SouthWestIndex = (l_X - 1) + (l_Y + 1) * in_Width;

		// East
		if(l_EastIndex <= l_Nodes.size()
			&& l_Nodes[l_EastIndex].x == l_X + 1 && l_Nodes[l_EastIndex].y == l_Y
			&& !in_GameGrid[l_EastIndex])
		{
		}
		// South-East
		if(l_SouthEastIndex <= l_Nodes.size()
			&& l_Nodes[l_SouthEastIndex].x == l_X + 1 && l_Nodes[l_SouthEastIndex].y == l_Y + 1
			&& !in_GameGrid[l_SouthEastIndex])
		{
		}
		// South
		if(l_SouthIndex <= l_Nodes.size()
			&& l_Nodes[l_SouthIndex].x == l_X && l_Nodes[l_SouthIndex].y == l_Y + 1
			&& !in_GameGrid[l_SouthIndex])
		{
		}
		// South-West
		if(l_SouthWestIndex <= l_Nodes.size()
			&& l_Nodes[l_SouthWestIndex].x == l_X - 1 && l_Nodes[l_SouthWestIndex].y == l_Y + 1
			&& !in_GameGrid[l_SouthWestIndex])
		{
		}

		if(++l_X > in_Width)
		{
			l_X = 0;
			++l_Y;
		}
	}

	return l_Ok;
}

Vector2 Navigator::GetBestDirection(const Vector2 & in_Position, const Vector2 & in_Goal) const
{
	return Vector2(0.f, 0.f);
}

void Navigator::UpdateHeuristic()
{

}
