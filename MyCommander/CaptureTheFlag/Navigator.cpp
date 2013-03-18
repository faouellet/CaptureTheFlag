#include "Navigator.h"

#include <algorithm>
#include <cassert>
#include <limits>

Navigator::Navigator(const std::unique_ptr<float[]> & in_Level, const int in_Length, const int in_Width,
					 const int in_MaxEntranceWidth) : 
m_BaseLevelLength(in_Length), m_BaseLevelWidth(in_Width), m_MaxEntranceWidth(in_MaxEntranceWidth) 
{
	int l_X = 0, l_Y = 0;
	std::for_each(in_Level.get(), in_Level.get() + m_BaseLevelLength * m_BaseLevelWidth, [&l_X, &l_Y, this](const float in_Block)
	{
		m_Clusters[0].push_back(Navigator::Cluster(0, 1, 1,
			std::vector<Navigator::Node>(1,Navigator::Node(0, static_cast<int>(in_Block), 
			Vector2(static_cast<float>(l_X), static_cast<float>(l_Y))))));
		if(l_X == m_BaseLevelLength)
		{
			l_X = 0;
			++l_Y;
		}
	});
}

// ********** Offline processing **********

void Navigator::AbstractMaze()
{
	BuildClusters(1);
	auto l_It1 = m_Clusters[1].begin();
	auto l_It2 = m_Clusters[1].begin() + 1;
	auto l_End = m_Clusters[1].end();
	
	while(l_It1 != l_End)
	{
		while(l_It2 != l_End)
		{
			Adjacency l_Adjacency;
			if(Adjacent(*l_It1, *l_It2, l_Adjacency))
			{
				BuildEntrances(*l_It1, *l_It2, 1, l_Adjacency);
			}
			++l_It2;
		}
		++l_It1;
		l_It2 = l_It1 + 1;
	}
}

// We assume that a cluster contains all nodes, even the ones representing a block
bool Navigator::Adjacent(const Cluster & in_Cluster1, const Cluster & in_Cluster2, Adjacency & out_Adjacency) const
{
	assert(in_Cluster1.Level == in_Cluster2.Level);
	bool l_Ok = false;
 
	// Cluster1 to the right of Cluster2 ?
	if(in_Cluster1.Nodes[0].Position.x == in_Cluster2.Nodes[in_Cluster2.Length].Position.x + 1)
	{
		l_Ok = true;
		out_Adjacency = Right;
	}
	// Cluster1 below Cluster2 ?
	else if(in_Cluster1.Nodes[0].Position.y == in_Cluster2.Nodes[in_Cluster2.Length * in_Cluster2.Width].Position.y - 1)
	{
		l_Ok = true;
		out_Adjacency = Below;
	}
	// Cluster1 to left of Cluster2 ?
	else if(in_Cluster1.Nodes[in_Cluster1.Length].Position.x == in_Cluster2.Nodes[0].Position.x - 1)
	{
		l_Ok = true;
		out_Adjacency = Left;
	}
	// Cluster1 above Cluster2 ?
	else if(in_Cluster1.Nodes[in_Cluster1.Length * in_Cluster1.Width].Position.y == in_Cluster2.Nodes[0].Position.y + 1)
	{
		l_Ok = true;
		out_Adjacency = Above;
	}

	return l_Ok;
}

void Navigator::BuildClusters(const int in_Level)
{
	// TODO : The levels seem to be 50x88, should confirm
	// TODO : Should nodes be added to the clusters here ?
	m_Clusters.push_back(std::vector<Cluster>(55, Cluster(1, 10, 8)));
}

void Navigator::BuildEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, const int in_Level, const Adjacency in_Adjacency)
{
	assert(in_Cluster1.Length == in_Cluster2.Length);
	assert(in_Cluster1.Width == in_Cluster2.Width);

	std::vector<std::pair<Node, Node>> l_Gates;

	// Describe th relationship between Cluster1 & Cluster2
	switch (in_Adjacency)
	{
		case Above:
		{
			BuildTopEntrances(in_Cluster1, in_Cluster2, l_Gates);
			break;
		}
		case Below:
		{
			BuildTopEntrances(in_Cluster2, in_Cluster1, l_Gates);
			break;
		}
		case Left:
		{
			BuildSideEntrances(in_Cluster1, in_Cluster2, l_Gates);
			break;
		}
		case Right:
		{
			BuildSideEntrances(in_Cluster2, in_Cluster1, l_Gates);
			break;
		}
		default:
		{
			break;
		}
	}

	m_Entrances[in_Level].push_back(Entrance(in_Cluster1, in_Cluster2, l_Gates));
}

void Navigator::BuildSideEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, std::vector<std::pair<Node, Node>> & out_Gates)
{
	int l_OpenSpaces = 0;
	int l_LastCutOff = 0;

	for(int i = 0; i < in_Cluster1.Width; ++i)
	{
		if(in_Cluster1.Nodes[in_Cluster1.Length * (in_Cluster1.Width * i) - 1].Height 
			&& in_Cluster2.Nodes[in_Cluster2.Width * i].Height)
		{
			if(++l_OpenSpaces == m_MaxEntranceWidth)
			{
				out_Gates.push_back(std::make_pair<Node, Node>(
					in_Cluster1.Nodes[in_Cluster1.Length * (in_Cluster2.Width * ((i + l_LastCutOff) / 2)) - 1],
					in_Cluster2.Nodes[in_Cluster2.Width * ((i + l_LastCutOff) / 2)]
					));
				l_OpenSpaces = 0;
				l_LastCutOff = i;
			}
		}
	}
}

void Navigator::BuildTopEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, std::vector<std::pair<Node, Node>> & out_Gates)
{
	int l_OpenSpaces = 0;
	int l_LastCutOff = 0;

	for(int i = 0; i < in_Cluster1.Length; ++i)
	{
		if(in_Cluster1.Nodes[in_Cluster1.Width * (in_Cluster1.Length - 1) + i].Height 
			&& in_Cluster2.Nodes[i].Height)
		{
			if(++l_OpenSpaces == m_MaxEntranceWidth)
			{
				out_Gates.push_back(std::make_pair<Node, Node>(
					in_Cluster1.Nodes[(in_Cluster1.Width * (in_Cluster1.Length - 1) + i + l_LastCutOff) / 2],
					in_Cluster2.Nodes[(i + l_LastCutOff) / 2]));
				l_OpenSpaces = 0;
				l_LastCutOff = i;
			}
		}
	}
}

void Navigator::BuildGraph()
{
	// TODO : Profile this to make sure it can be done within 5 seconds
	std::for_each(m_Entrances[0].begin(), m_Entrances[0].end(), [this](const Entrance & in_Entrance)
	{
		auto l_Cluster1 = std::find(m_Clusters[1].begin(), m_Clusters[1].end(), in_Entrance.Clusters.first);
		auto l_Cluster2 = std::find(m_Clusters[1].begin(), m_Clusters[1].end(), in_Entrance.Clusters.second);

		std::for_each(in_Entrance.Gates.begin(), in_Entrance.Gates.end(), [&l_Cluster1, &l_Cluster2](std::pair<Node,Node> in_Gate)
		{
			l_Cluster1->Nodes.push_back(in_Gate.first);
			l_Cluster2->Nodes.push_back(in_Gate.second);

			l_Cluster1->Edges.push_back(Edge(in_Gate.first, in_Gate.second, 1, 1.0, Inter));
			l_Cluster1->Edges.push_back(Edge(in_Gate.second, in_Gate.first, 1, 1.0, Inter));
		});
	});
	AddIntraEdgesToClusters(1);
}

// TODO : This function and the one it calls(SearchForDistance) should be interruptible
void Navigator::AddIntraEdgesToClusters(const int in_Level)
{
	std::for_each(m_Clusters[in_Level].begin(), m_Clusters[in_Level].end(), [this, &in_Level](Cluster & in_Cluster)
	{
		auto l_It1 = in_Cluster.Nodes.begin();
		auto l_It2 = in_Cluster.Nodes.begin() + 1;
		auto l_End = in_Cluster.Nodes.end();
	
		while(l_It1 != l_End)
		{
			while(l_It2 != l_End)
			{
				double l_Distance = SearchForDistance(*l_It1, *l_It2, in_Cluster);

				if(l_Distance < std::numeric_limits<double>::infinity())
				{
					in_Cluster.Edges.push_back(Navigator::Edge(*l_It1, *l_It2, in_Level, l_Distance, Intra));
				}
			}
			++l_It1;
			l_It2 = l_It1 + 1;
		}
	});
}

void Navigator::Preprocess()
{
	AbstractMaze();
	BuildGraph();
}

// ********** Online processing **********

void Navigator::ConnectToBorder(const Navigator::Node & in_Node, Cluster & in_Cluster)
{
	for(auto l_It = in_Cluster.Nodes.begin(); l_It != in_Cluster.Nodes.end(); ++l_It)
	{
		if(in_Node.Level < in_Cluster.Level)
			continue;

		double l_Distance = SearchForDistance(in_Node, *l_It, in_Cluster);

		if(l_Distance < std::numeric_limits<double>::infinity())
		{
			in_Cluster.Edges.push_back(Edge(in_Node, *l_It, in_Cluster.Level, l_Distance, Intra));
		}
	}
}

void Navigator::InsertNode(Navigator::Node && in_Node, const int in_Level)
{
	for(int i = 1; i <= in_Level; ++i)
	{
		auto l_It = m_Clusters[in_Level].begin();
		for(; l_It != m_Clusters[in_Level].end(); ++l_It)
		{
			if(std::find(l_It->Nodes.begin(), l_It->Nodes.end(), in_Node) != l_It->Nodes.end())
				break;
		}

		ConnectToBorder(in_Node, *l_It);
	}
}

double Navigator::SearchForDistance(const Node & in_Node1, const Node & in_Node2, const Cluster & in_Cluster) const
{
	// Can't find the distance if one of the given nodes is a block
	if(in_Node1.Height || in_Node2.Height)
		return std::numeric_limits<double>::infinity();


}

// TODO : Vector2 or Node for args ?
Vector2 Navigator::GetBestDirection(const Vector2 & in_Start, const Vector2 & in_Goal, const int in_Level)
{
	InsertNode(Node(in_Level, 0, in_Start), in_Level);
	InsertNode(Node(in_Level, 0, in_Goal), in_Level);

}

void Navigator::UpdateHeuristic()
{

}
