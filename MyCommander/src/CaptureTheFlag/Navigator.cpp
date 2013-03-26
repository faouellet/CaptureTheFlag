/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*/

#include "Navigator.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <set>

#include "Heuristics.h"

const int Navigator::M_MAXCLUSTERSIZE = 20;

void Navigator::Init(const std::unique_ptr<float[]> & in_Level, const int in_Length, const int in_Width,
					 const int in_MaxEntranceWidth)
{
	int l_X = 0, l_Y = 0;
	m_Clusters.push_back(std::vector<Cluster>(1, Cluster()));
	auto l_Cluster = m_Clusters[0].begin();
	std::for_each(in_Level.get(), in_Level.get() + in_Length * in_Width, [&l_Cluster, &in_Length, &l_X, &l_Y, this](const float in_Block)
	{
		// The first level cluster is in fact the whole map
		l_Cluster->Nodes.push_back(std::make_shared<Node>(Navigator::Node(0, static_cast<int>(in_Block), Vector2(static_cast<float>(l_X), static_cast<float>(l_Y)))));
		
		if(l_X < in_Length - 1)
		{
			++l_X;
		}
		else
		{
			l_X = 0;
			++l_Y;
		}
	});

	// Construct the low level cluster local graph
	int l_CurrentNodeIndex = 0;	
	for(int i  = 0; i < in_Width; ++i)
	{
		for(int j = 0; j < in_Length; ++j)
		{
			l_CurrentNodeIndex = j + i * in_Width;
			if(!l_Cluster->Nodes[l_CurrentNodeIndex]->Height)
			{
				// Link the right node
				if(j < in_Length-1 && !l_Cluster->Nodes[l_CurrentNodeIndex+1]->Height)
				{
					l_Cluster->LocalGraph[l_Cluster->Nodes[l_CurrentNodeIndex]][l_Cluster->Nodes[l_CurrentNodeIndex+1]] = 1.0;
					l_Cluster->LocalGraph[l_Cluster->Nodes[l_CurrentNodeIndex+1]][l_Cluster->Nodes[l_CurrentNodeIndex]] = 1.0;
				}
				// Link the down-right node
				if(j < in_Length-1 && i < in_Width-1 && !l_Cluster->Nodes[(j+1) + (i+1) * in_Width]->Height)
				{
					l_Cluster->LocalGraph[l_Cluster->Nodes[l_CurrentNodeIndex]][l_Cluster->Nodes[(j+1) + (i+1) * in_Width]] = 1.0;
					l_Cluster->LocalGraph[l_Cluster->Nodes[(j+1) + (i+1) * in_Width]][l_Cluster->Nodes[l_CurrentNodeIndex]] = 1.0;
				}
				// Link the down node
				if(i < in_Width-1 && !l_Cluster->Nodes[j + (i+1) * in_Width]->Height)
				{
					l_Cluster->LocalGraph[l_Cluster->Nodes[l_CurrentNodeIndex]][l_Cluster->Nodes[j + (i+1) * in_Width]] = 1.0;
					l_Cluster->LocalGraph[l_Cluster->Nodes[j + (i+1) * in_Width]][l_Cluster->Nodes[l_CurrentNodeIndex]] = 1.0;
				}
				// Link the down-left node
				if(j > 0 && i < in_Width-1 && !l_Cluster->Nodes[(j-1) + (i+1) * in_Width]->Height)
				{
					l_Cluster->LocalGraph[l_Cluster->Nodes[l_CurrentNodeIndex]][l_Cluster->Nodes[(j-1) + (i+1) * in_Width]] = 1.0;
					l_Cluster->LocalGraph[l_Cluster->Nodes[(j-1) + (i+1) * in_Width]][l_Cluster->Nodes[l_CurrentNodeIndex]] = 1.0;
				}
			}
		}
	}
		
	m_Graphs.push_back(l_Cluster->LocalGraph);
	m_Clusters[0].begin()->Length = in_Length;
	m_Clusters[0].begin()->Width = in_Width;
	m_MaxEntranceWidth = in_MaxEntranceWidth;

	Preprocess();
}

double Navigator::AStar(const std::shared_ptr<Node> & in_Start, const std::shared_ptr<Node> & in_Goal, const int in_Level, const IHeuristic & in_Heuristic)
{
	std::set<std::pair<std::shared_ptr<Node>, double>, Comparator> l_Closed;
	std::set<std::pair<std::shared_ptr<Node>, double>, Comparator> l_Opened;
	l_Opened.insert(std::make_pair<std::shared_ptr<Node>, double>(in_Start, 0.0));
	std::map<std::shared_ptr<Node>, std::shared_ptr<Node>> l_Parents;
	l_Parents[in_Start] = std::make_shared<Node>(Node());
	std::map<std::shared_ptr<Node>, double> l_RealCosts;
	std::vector<std::shared_ptr<Node>> l_Path;

	// TODO : Yeaaahh.... about that condition...
	while(true)
	{
		if(l_Opened.empty())
			return std::numeric_limits<double>::infinity();

		// TODO : Cache the call l_Opened.begin() ?
		std::shared_ptr<Node> l_Node = l_Opened.begin()->first;
		l_Closed.insert(*l_Opened.begin());
		l_Opened.erase(l_Opened.begin());

		if(l_Node == in_Goal)
		{
			// Reverse the parents' chain to get the path
			std::shared_ptr<Node> l_ParentNode = l_Parents[l_Node];

			l_Path.push_back(l_Node);
			 
			while(l_ParentNode != in_Start)
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
		(const std::map<std::shared_ptr<Node>, double>::value_type & in_Val)
		{
			// Transition between diagonal nodes is 1.42 and 1 for vertical/horizontal nodes
			double l_TransitionCost = (in_Val.first->Position.x == l_Node->Position.x 
				|| in_Val.first->Position.y == l_Node->Position.y) ? 1.0 : 1.42;

			l_RealCosts[in_Val.first] = l_RealCosts[l_Node] + l_TransitionCost;
			double l_ValCost = l_RealCosts[in_Val.first] + in_Heuristic(*in_Val.first, *in_Goal);
			l_Parents[in_Val.first] = l_Node;
			
			// TODO : Merge the two loops ? Or at least don't repeat this much code...
			for(auto l_OpenedIt = l_Opened.begin(); l_OpenedIt != l_Opened.end(); ++l_OpenedIt)
			{
				if(l_OpenedIt->first == in_Val.first)
				{
					double l_NeighborCost = l_RealCosts[l_OpenedIt->first] + in_Heuristic(*l_OpenedIt->first, *in_Goal);
					if(l_ValCost <= l_NeighborCost)
					{
						l_Opened.erase(l_OpenedIt);
						break;
					}					
				}
			}

			for(auto l_ClosedIt = l_Closed.begin(); l_ClosedIt != l_Closed.end(); ++l_ClosedIt)
			{
				if(l_ClosedIt->first == in_Val.first)
				{					
					double l_NeighborCost = l_RealCosts[l_ClosedIt->first] + in_Heuristic(*l_ClosedIt->first, *in_Goal);
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
		if(l_It1 != l_End)
			l_It2 = l_It1 + 1;
	}
}

// We assume that a cluster contains all nodes, even the ones representing a block
bool Navigator::Adjacent(const Cluster & in_Cluster1, const Cluster & in_Cluster2, Adjacency & out_Adjacency) const
{
	assert(in_Cluster1.Level == in_Cluster2.Level);
	bool l_Ok = false;
 
	// Cluster1 to the right of Cluster2 ?
	if(in_Cluster1.Nodes[0]->Position.x == in_Cluster2.Nodes[in_Cluster2.Length-1]->Position.x+1
		&& in_Cluster1.Nodes[0]->Position.y == in_Cluster2.Nodes[in_Cluster2.Length-1]->Position.y)
	{
		l_Ok = true;
		out_Adjacency = Right;
	}
	// Cluster1 below Cluster2 ?
	else if(in_Cluster1.Nodes[0]->Position.y == in_Cluster2.Nodes[(in_Cluster2.Length-1)*in_Cluster2.Width]->Position.y+1
		&& in_Cluster1.Nodes[0]->Position.x == in_Cluster2.Nodes[(in_Cluster2.Length-1)*in_Cluster2.Width]->Position.x)
	{
		l_Ok = true;
		out_Adjacency = Below;
	}
	// Cluster1 to left of Cluster2 ?
	else if(in_Cluster1.Nodes[in_Cluster1.Length-1]->Position.x == in_Cluster2.Nodes[0]->Position.x-1
		&& in_Cluster1.Nodes[in_Cluster1.Length-1]->Position.y == in_Cluster2.Nodes[0]->Position.y)
	{
		l_Ok = true;
		out_Adjacency = Left;
	}
	// Cluster1 above Cluster2 ?
	else if(in_Cluster1.Nodes[(in_Cluster1.Length-1)*in_Cluster1.Width]->Position.y == in_Cluster2.Nodes[0]->Position.y-1
		&& in_Cluster1.Nodes[(in_Cluster1.Length-1)*in_Cluster1.Width]->Position.x == in_Cluster2.Nodes[0]->Position.x)
	{
		l_Ok = true;
		out_Adjacency = Above;
	}

	return l_Ok;
}

int Navigator::ClusterSize(const int in_Size) const
{
	int l_ClusterSize = M_MAXCLUSTERSIZE;
	while(l_ClusterSize > 0)
	{
		if(in_Size % l_ClusterSize || l_ClusterSize == in_Size)
			--l_ClusterSize;
		else
			break;
	}

	return l_ClusterSize;
}

// TODO : Only works for the first layer of clusters for now
void Navigator::BuildClusters(const int in_Level)
{
	int l_Length = m_Clusters[0].begin()->Length;
	int l_Width = m_Clusters[0].begin()->Width;

	int l_ClusterLength = ClusterSize(l_Length);
	int l_ClusterWidth = ClusterSize(l_Width);

	m_Clusters.push_back(std::vector<Cluster>(
		(l_Length / l_ClusterLength) * (l_Width / l_ClusterWidth),
		Cluster(1, l_ClusterLength, l_ClusterWidth)));
	m_Entrances.push_back(std::vector<Entrance>());

	int l_LengthOffset = 0;
	int l_WidthOffset = 0;

	for(auto l_It = m_Clusters[in_Level].begin(); l_It != m_Clusters[in_Level].end(); ++l_It)
	{
		for(int i = l_WidthOffset; i < l_ClusterWidth + l_WidthOffset; ++i)
		{
			for(int j = l_LengthOffset; j < l_ClusterLength + l_LengthOffset; ++j)
			{
				l_It->Nodes.push_back(m_Clusters[in_Level - 1].begin()->Nodes[j + (i * l_Width)]);
			}
		}
		if(l_LengthOffset + l_ClusterLength < l_Length)
		{
			l_LengthOffset += l_ClusterLength;	
		}
		else
		{
			l_LengthOffset = 0;
			l_WidthOffset += l_ClusterWidth;
		}
	}
}

void Navigator::BuildEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, const int in_Level, const Adjacency in_Adjacency)
{
	assert(in_Cluster1.Length == in_Cluster2.Length);
	assert(in_Cluster1.Width == in_Cluster2.Width);

	std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> l_Gates;

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

	m_Entrances[in_Level-1].push_back(Entrance(in_Cluster1, in_Cluster2, l_Gates));
}

void Navigator::BuildSideEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, 
								   std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> & out_Gates)
{
	std::vector<std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>>> l_PotentialGates(in_Cluster1.Width);
	int j = 0;

	for(int i = 0; i < in_Cluster1.Width; ++i)
	{
		int l_Index1 = in_Cluster1.Length + (in_Cluster1.Width * i) - 1;
		int l_Index2 = in_Cluster2.Width * i;
		if(in_Cluster1.Nodes[l_Index1]->Height 
			|| in_Cluster2.Nodes[l_Index2]->Height)
		{
			++j;
		}
		else
		{
			l_PotentialGates[j].push_back(std::make_pair(
				std::make_shared<Node>(Node(in_Cluster1.Level, in_Cluster1.Nodes[l_Index1]->Height, in_Cluster1.Nodes[l_Index1]->Position)),
				std::make_shared<Node>(Node(in_Cluster2.Level, in_Cluster2.Nodes[l_Index2]->Height, in_Cluster2.Nodes[l_Index2]->Position))));
		}
	}

	for(int i = 0; i <= j; ++i)
	{
		if(l_PotentialGates[i].size() == m_MaxEntranceWidth)
		{
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()/2]);
		}
		else if(l_PotentialGates[i].size() < m_MaxEntranceWidth)
		{
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1]);
		}
		else // l_PotentialGates[i].size() > m_MaxEntranceWidth
		{
			out_Gates.push_back(l_PotentialGates[i][0]);
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1]);
		}
	}
}

void Navigator::BuildTopEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, 
								  std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> & out_Gates)
{
	std::vector<std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>>> l_PotentialGates(in_Cluster1.Width);
	int j = 0;

	for(int i = 0; i < in_Cluster1.Length; ++i)
	{
		if(in_Cluster1.Nodes[in_Cluster1.Width * (in_Cluster1.Length - 1) + i]->Height 
			|| in_Cluster2.Nodes[i]->Height)
		{
			++j;
		}
		else
		{
			l_PotentialGates[j].push_back(std::make_pair(
				in_Cluster1.Nodes[in_Cluster1.Width*(in_Cluster1.Length-1)+i],
				in_Cluster2.Nodes[i]));
		}
	}

	for(int i = 0; i <= j; ++i)
	{
		if(l_PotentialGates[i].size() == m_MaxEntranceWidth)
		{
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()/2]);
		}
		else if(l_PotentialGates[i].size() < m_MaxEntranceWidth)
		{
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1]);
		}
		else // l_PotentialGates[i].size() > m_MaxEntranceWidth
		{
			out_Gates.push_back(l_PotentialGates[i][0]);
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1]);
		}
	}
}

void Navigator::BuildGraph()
{
	m_Graphs.push_back(std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, double>>());

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
				++l_It2;
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

void Navigator::ConnectToBorder(const std::shared_ptr<Node> & in_Node, Cluster & in_Cluster)
{
	for(auto l_It = in_Cluster.Nodes.begin(); l_It != in_Cluster.Nodes.end(); ++l_It)
	{
		if(in_Node->Level < in_Cluster.Level)
			continue;

		double l_Distance = SearchForDistance(in_Node, *l_It, in_Cluster);

		if(l_Distance < std::numeric_limits<double>::infinity())
		{
			in_Cluster.LocalGraph[in_Node][*l_It] = l_Distance;
		}
	}
}

void Navigator::InsertNode(const std::shared_ptr<Node> & in_Node, const int in_Level)
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

double Navigator::SearchForDistance(const std::shared_ptr<Node> & in_Node1, const std::shared_ptr<Node> & in_Node2, const Cluster & in_Cluster)
{
	// Can't find the distance if one of the given nodes is a block
	if(in_Node1->Height || in_Node2->Height)
		return std::numeric_limits<double>::infinity();

	return AStar(in_Node1, in_Node2, in_Cluster.Level-1, TrivialHeuristic());
}

Vector2 Navigator::GetBestDirection(const Vector2 & in_Start, const Vector2 & in_Goal)
{
	// TODO : Do we want to scale up or no ? Because this doesn't scale
	std::shared_ptr<Node> l_StartNode = std::make_shared<Node>(Node(1, 0, in_Start));
	std::shared_ptr<Node> l_EndNode = std::make_shared<Node>(Node(1, 0, in_Goal));

	InsertNode(l_StartNode, 1);
	InsertNode(l_EndNode, 1);

	if(AStar(l_StartNode, l_EndNode, 1, EuclideanDistance()) < std::numeric_limits<double>::infinity())
	{

	}

	// TODO : Smooth/Refine path ?

	return in_Goal;
}
