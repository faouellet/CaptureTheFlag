#include "Navigator.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <set>

#include "Heuristics.h"

void Navigator::Init(const std::unique_ptr<float[]> & in_Level, const int in_Length, const int in_Width,
					 const int in_MaxEntranceWidth)
{
	int l_X = 0, l_Y = 0;
	std::for_each(in_Level.get(), in_Level.get() + in_Length * in_Width, [&in_Length, &l_X, &l_Y, this](const float in_Block)
	{
		// The first level cluster is in fact the whole map
		Node l_Node(0, static_cast<int>(in_Block), Vector2(static_cast<float>(l_X), static_cast<float>(l_Y)));
		m_Clusters[0].begin()->Nodes.push_back(l_Node);
		// TODO : Construct the low level graph and the low level cluster local graph
		// m_Graphs[0].insert(l_Node);

		if(l_X == in_Length)
		{
			l_X = 0;
			++l_Y;
		}
	});
	m_Clusters[0].begin()->Length = in_Length;
	m_Clusters[0].begin()->Width = in_Width;
	m_MaxEntranceWidth =in_MaxEntranceWidth;
}

double Navigator::AStar(const Node & in_Start, const Node & in_Goal, const int in_Level, const IHeuristic & in_Heuristic)
{
	std::set<std::pair<Node, double>, Comparator> l_Closed;
	std::set<std::pair<Node, double>, Comparator> l_Opened;
	l_Opened.insert(std::make_pair<Node, double>(in_Start, 0.0));
	std::map<Node, Node> l_Parents;
	l_Parents[in_Start] = Node();
	std::map<Node, double> l_RealCosts;
	std::vector<Node> l_Path;

	// TODO : Yeaaahh.... about that condition...
	while(true)
	{
		if(l_Opened.empty())
			return std::numeric_limits<double>::infinity();

		// TODO : Cache the call l_Opened.begin() ?
		Node l_Node = l_Opened.begin()->first;
		l_Closed.insert(*l_Opened.begin());
		l_Opened.erase(l_Opened.begin());

		if(l_Node == in_Goal)
		{
			// Reverse the parents' chain to get the path
			Node l_ParentNode = l_Parents[l_Node];
			Node l_EndNode;

			l_Path.push_back(l_Node);
			 
			while(l_ParentNode != l_EndNode)
			{
				l_Path.push_back(l_ParentNode);
				l_ParentNode = l_Parents[l_ParentNode];
			}
			// TODO : Use std::deque instead of reversing a vector ?
			std::reverse(l_Path.begin(), l_Path.end());

			// Cache the path
			m_Paths[in_Start][in_Goal] = l_Path;

			return l_RealCosts[l_Node];
		}

		std::for_each(m_Graphs[in_Level].at(l_Node).begin(), m_Graphs[in_Level].at(l_Node).end(), 
			[&l_Closed, &l_Opened, &l_RealCosts, &l_Node,& l_Parents, &in_Heuristic, &in_Goal]
		(const std::map<Node, double>::value_type & in_Val)
		{
			// Transition between diagonal nodes is 1.42 and 1 for vertical/horizontal nodes
			double l_TransitionCost = (in_Val.first.Position.x == l_Node.Position.x 
				|| in_Val.first.Position.y == l_Node.Position.y) ? 1.0 : 1.42;

			l_RealCosts[in_Val.first] = l_RealCosts[l_Node] + l_TransitionCost;
			double l_ValCost = l_RealCosts[in_Val.first] + in_Heuristic(in_Val.first, in_Goal);
			l_Parents[in_Val.first] = l_Node;
			
			// TODO : Merge the two loops ? Or at least don't repeat this much code...
			for(auto l_OpenedIt = l_Opened.begin(); l_OpenedIt != l_Opened.end(); ++l_OpenedIt)
			{
				if(l_OpenedIt->first == in_Val.first)
				{
					double l_NeighborCost = l_RealCosts[l_OpenedIt->first] + in_Heuristic(l_OpenedIt->first, in_Goal);
					if(l_ValCost <= l_NeighborCost)
					{
						l_Opened.erase(l_OpenedIt);
						break;
					}					
				}
			}

			for(auto l_ClosedIt = l_Opened.begin(); l_ClosedIt != l_Opened.end(); ++l_ClosedIt)
			{
				if(l_ClosedIt->first == in_Val.first)
				{					
					double l_NeighborCost = l_RealCosts[l_ClosedIt->first] + in_Heuristic(l_ClosedIt->first, in_Goal);
					if(l_ValCost <= l_NeighborCost)
					{
						l_Closed.erase(l_ClosedIt);
						break;
					}					
				}
			}
			
			l_Opened.insert(std::make_pair(in_Val.first, l_ValCost));
		});
	}
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
	// TODO : Find a generic way to compute cluster's length and width
	// TODO : Only works for the first layer of cluster for now
	m_Clusters.push_back(std::vector<Cluster>(55, Cluster(1, 10, 8)));

	int l_LengthCpt = 0;
	int l_WidthCpt = 0;
	std::for_each(m_Clusters[in_Level-1].begin()->Nodes.begin(), m_Clusters[in_Level-1].begin()->Nodes.end(), 
		[&l_LengthCpt, &l_WidthCpt, this](const Node & in_Node)
	{
		//m_Clusters[0].begin()->;
	});
}

void Navigator::BuildEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, const int in_Level, const Adjacency in_Adjacency)
{
	assert(in_Cluster1.Length == in_Cluster2.Length);
	assert(in_Cluster1.Width == in_Cluster2.Width);

	std::vector<std::pair<Node, Node>> l_Gates;

	// Describe the relationship between Cluster1 & Cluster2
	switch (in_Adjacency)
	{
		case Above:
			BuildTopEntrances(in_Cluster1, in_Cluster2, l_Gates);
			break;
		case Below:
			BuildTopEntrances(in_Cluster2, in_Cluster1, l_Gates);
			break;
		case Left:
			BuildSideEntrances(in_Cluster1, in_Cluster2, l_Gates);
			break;
		case Right:
			BuildSideEntrances(in_Cluster2, in_Cluster1, l_Gates);
			break;
		default:
			break;
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

		for(auto l_GatesIt = in_Entrance.Gates.begin(); l_GatesIt != in_Entrance.Gates.end(); ++l_GatesIt)
		{
			// High level transitions always cost 1
			m_Graphs[1][l_GatesIt->first][l_GatesIt->second] = 1.0;
			m_Graphs[1][l_GatesIt->second][l_GatesIt->first] = 1.0;
		}
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
					in_Cluster.LocalGraph[*l_It1][*l_It2] = l_Distance;
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
			in_Cluster.LocalGraph[in_Node][*l_It] = l_Distance;
		}
	}
}

void Navigator::InsertNode(Navigator::Node & in_Node, const int in_Level)
{
	for(int i = 1; i <= in_Level; ++i)
	{
		auto l_It = m_Clusters[in_Level].begin();
		for(; l_It != m_Clusters[in_Level].end(); ++l_It)
		{
			if(std::find(l_It->Nodes.begin(), l_It->Nodes.end(), in_Node) != l_It->Nodes.end())
				break;
		}

		if(l_It != m_Clusters[in_Level].end())
			ConnectToBorder(in_Node, *l_It);
		// TODO : Else ?
	}
}

double Navigator::SearchForDistance(const Node & in_Node1, const Node & in_Node2, const Cluster & in_Cluster)
{
	// Can't find the distance if one of the given nodes is a block
	if(in_Node1.Height || in_Node2.Height)
		return std::numeric_limits<double>::infinity();

	return AStar(in_Node1, in_Node2, in_Cluster.Level, TrivialHeuristic());
}

Vector2 Navigator::GetBestDirection(const Vector2 & in_Start, const Vector2 & in_Goal)
{
	// TODO : Do we want to scale up or no ? Because this doesn't scale
	Node l_StartNode = Node(1, 0, in_Start);
	Node l_EndNode = Node(1, 0, in_Goal);

	InsertNode(l_StartNode, 1);
	InsertNode(l_EndNode, 1);

	if(AStar(l_StartNode, l_EndNode, 1, EuclideanDistance()) < std::numeric_limits<double>::infinity())
	{

	}

	// TODO : Smooth/Refine path ?
}
