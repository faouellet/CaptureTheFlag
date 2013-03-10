#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <memory>
#include <utility>
#include <vector>

#include "boost\graph\adjacency_list.hpp"
#include "boost\graph\graph_traits.hpp"

#include "api\GameInfo.h"
#include "api\Vector2.h"

/*
* Navigator
* Construct a navigation graph out of the map data and plan the shortest safe
* distance toward a goal for the bots in this navigation graph
*/
class Navigator
{
public:
	typedef Vector2 Node;
	typedef std::pair<Node,Node> Edge;
	typedef boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS, Node, Edge> Graph;
	typedef Graph::vertex_descriptor VertexID;
	typedef Graph::edge_descriptor EdgeID;

private:
	Graph m_CellGrid;
	std::vector<Graph> m_SectorGrids;
	// TODO : Something for the heuristic?
	// TODO : Internal helper functions?

public:
	bool Init(std::unique_ptr<float[]> & in_GameGrid, const int in_Width);
	Vector2 GetBestDirection(const Vector2 & in_Position, const Vector2 & in_Goal) const;
	void UpdateHeuristic();

};

#endif // NAVIGATOR_H
