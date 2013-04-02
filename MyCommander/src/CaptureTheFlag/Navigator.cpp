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
#include "Resumable.h"

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
		l_Cluster->LevelNodes.push_back(std::make_shared<Node>(Navigator::Node(0, static_cast<int>(in_Block), Vector2(static_cast<float>(l_X), static_cast<float>(l_Y)))));
		
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
		
	BuildLocalGraph(in_Length, in_Width, l_Cluster->LevelNodes, l_Cluster->LocalGraph);

	m_Graphs.push_back(l_Cluster->LocalGraph);
	m_Clusters[0].begin()->Length = in_Length;
	m_Clusters[0].begin()->Width = in_Width;
	m_MaxEntranceWidth = in_MaxEntranceWidth;

	Preprocess();
}

void Navigator::Reset()
{
	m_Clusters.clear();
	m_Graphs.clear();
	m_Entrances.clear();
	m_Paths.clear();
}

double Navigator::AStar(const std::shared_ptr<Node> & in_Start, const std::shared_ptr<Node> & in_Goal, 
						Graph & in_Graph, const IHeuristic & in_Heuristic)
{
	if(in_Start->Height || in_Goal->Height || in_Start == in_Goal)
		return std::numeric_limits<double>::infinity();

	std::multimap<double, std::shared_ptr<Node>> l_Closed;
	std::multimap<double, std::shared_ptr<Node>> l_Opened;
	l_Opened.insert(std::make_pair(0.0, in_Start));
	std::map<std::shared_ptr<Node>, std::shared_ptr<Node>> l_Parents;
	l_Parents[in_Start] = std::make_shared<Node>(Node());
	
	std::map<std::shared_ptr<Node>, double> l_RealCosts;
	l_RealCosts[l_Opened.begin()->second] = 0.0;
	
	while(!l_Opened.empty())
	{
		std::shared_ptr<Node> l_CurrentNode = l_Opened.begin()->second;
		l_Closed.insert(*l_Opened.begin());
		l_Opened.erase(l_Opened.begin());

		if(l_CurrentNode == in_Goal)
		{
			std::vector<std::shared_ptr<Node>> l_Path;
			
			// Reverse the parents' chain to get the path
			std::shared_ptr<Node> l_ParentNode = l_Parents[l_CurrentNode];

			l_Path.push_back(l_CurrentNode);
			 
			while(l_ParentNode != in_Start)
			{
				l_Path.push_back(l_ParentNode);
				l_ParentNode = l_Parents[l_ParentNode];
			}
			l_Path.push_back(in_Start);
			std::reverse(l_Path.begin(), l_Path.end());

			// Cache the path
			m_Paths[in_Start][in_Goal] = l_Path;

			return l_RealCosts[l_CurrentNode];
		}

		if(in_Graph[l_CurrentNode] != std::map<std::shared_ptr<Node>, double, NodeComparer>())
		{
			for(auto l_NeighborIt = in_Graph.at(l_CurrentNode).begin(); l_NeighborIt != in_Graph.at(l_CurrentNode).end(); ++l_NeighborIt)
			{
				// Transition between diagonal nodes is 1.42 and 1 for vertical/horizontal nodes
				double l_TransitionCost = (l_NeighborIt->first->Position.x == l_CurrentNode->Position.x 
					|| l_NeighborIt->first->Position.y == l_CurrentNode->Position.y) ? 1.0 : 1.42;

				double l_TentativeRealCost = l_RealCosts[l_CurrentNode] + l_TransitionCost;

				auto l_ClosedNodeIt = std::find_if(l_Closed.begin(), l_Closed.end(), 
					[&l_NeighborIt](const std::pair<double, std::shared_ptr<Node>> & in_Node)
				{
					return *in_Node.second == *l_NeighborIt->first;
				});

				if(l_ClosedNodeIt != l_Closed.end() && l_TentativeRealCost >= l_RealCosts[l_NeighborIt->first])
					continue;

				auto l_OpenedIt = std::find_if(l_Opened.begin(), l_Opened.end(), 
					[&l_NeighborIt](const std::pair<double, std::shared_ptr<Node>> & in_Node)
				{
					return *in_Node.second == *l_NeighborIt->first;
				});

				if(l_OpenedIt == l_Opened.end() || l_TentativeRealCost < l_RealCosts[l_NeighborIt->first])
				{
					l_Parents[l_NeighborIt->first] = l_CurrentNode;
					l_RealCosts[l_NeighborIt->first] = l_TentativeRealCost;
					if(l_OpenedIt == l_Opened.end())
					{
						l_Opened.insert(std::make_pair(l_RealCosts[l_NeighborIt->first] + in_Heuristic(*l_NeighborIt->first, *in_Goal), l_NeighborIt->first));
					}
				}
			}
		}
	}

	return std::numeric_limits<double>::infinity();
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
// We also assume that we are only gonna call this on the first layer
bool Navigator::Adjacent(const Cluster & in_Cluster1, const Cluster & in_Cluster2, Adjacency & out_Adjacency) const
{
	assert(in_Cluster1.Level == in_Cluster2.Level);
	bool l_Ok = false;
 
	// Cluster1 to the right of Cluster2 ?
	if(in_Cluster1.BaseNodes[0]->Position.IsAtTheRightOf(in_Cluster2.BaseNodes[in_Cluster2.Length-1]->Position))
	{
		l_Ok = true;
		out_Adjacency = Right;
	}
	// Cluster1 below Cluster2 ?
	else if(in_Cluster1.BaseNodes[0]->Position.IsBelow(in_Cluster2.BaseNodes[(in_Cluster2.Length-1)*in_Cluster2.Width]->Position))
	{
		l_Ok = true;
		out_Adjacency = Below;
	}
	// Cluster1 to left of Cluster2 ?
	else if(in_Cluster1.BaseNodes[in_Cluster1.Length-1]->Position.IsAtTheLeftOf(in_Cluster2.BaseNodes[0]->Position))
	{
		l_Ok = true;
		out_Adjacency = Left;
	}
	// Cluster1 above Cluster2 ?
	else if(in_Cluster1.BaseNodes[(in_Cluster1.Length-1)*in_Cluster1.Width]->Position.IsAbove(in_Cluster2.BaseNodes[0]->Position))
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

void Navigator::BuildLocalGraph(const int in_Length, const int in_Width, const std::vector<std::shared_ptr<Node>> & in_Nodes, Graph & in_LocalGraph)
{
	int l_CurrentNodeIndex = 0;	
	int l_RightNodeIndex = 0;
	int l_DownRightIndex = 0;
	int l_DownIndex = 0;
	int l_DownLeftIndex = 0;

	for(int i  = 0; i < in_Width; ++i)
	{
		for(int j = 0; j < in_Length; ++j)
		{
			l_CurrentNodeIndex = j + i * in_Length;
			l_RightNodeIndex = l_CurrentNodeIndex+1;
			l_DownRightIndex = (j+1) + (i+1) * in_Length;
			l_DownIndex = j + (i+1) * in_Length;
			l_DownLeftIndex = (j-1) + (i+1) * in_Length;
			if(!in_Nodes[l_CurrentNodeIndex]->Height)
			{
				// Link the right node
				if(j < in_Length-1 && !in_Nodes[l_CurrentNodeIndex+1]->Height)
				{
					in_LocalGraph[in_Nodes[l_CurrentNodeIndex]][in_Nodes[l_RightNodeIndex]] = 1.0;
					in_LocalGraph[in_Nodes[l_RightNodeIndex]][in_Nodes[l_CurrentNodeIndex]] = 1.0;
				}
				// Link the down-right node
				if(j < in_Length-1 && i < in_Width-1 && !in_Nodes[l_DownRightIndex]->Height)
				{
					in_LocalGraph[in_Nodes[l_CurrentNodeIndex]][in_Nodes[l_DownRightIndex]] = 1.0;
					in_LocalGraph[in_Nodes[l_DownRightIndex]][in_Nodes[l_CurrentNodeIndex]] = 1.0;
				}
				// Link the down node
				if(i < in_Width-1 && !in_Nodes[l_DownIndex]->Height)
				{
					in_LocalGraph[in_Nodes[l_CurrentNodeIndex]][in_Nodes[l_DownIndex]] = 1.0;
					in_LocalGraph[in_Nodes[l_DownIndex]][in_Nodes[l_CurrentNodeIndex]] = 1.0;
				}
				// Link the down-left node
				if(j > 0 && i < in_Width-1 && !in_Nodes[l_DownLeftIndex]->Height)
				{
					in_LocalGraph[in_Nodes[l_CurrentNodeIndex]][in_Nodes[l_DownLeftIndex]] = 1.0;
					in_LocalGraph[in_Nodes[l_DownLeftIndex]][in_Nodes[l_CurrentNodeIndex]] = 1.0;
				}
			}
		}
	}
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
				l_It->BaseNodes.push_back(m_Clusters[in_Level - 1].begin()->LevelNodes[j + (i * l_Length)]);
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

void Navigator::BuildEntrances(Cluster & in_Cluster1, Cluster & in_Cluster2, const int in_Level, const Adjacency in_Adjacency)
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

void Navigator::BuildSideEntrances(Cluster & in_Cluster1, Cluster & in_Cluster2, std::vector<Gate> & out_Gates)
{
	std::vector<std::vector<Gate>> l_PotentialGates(in_Cluster1.Width);
	int j = 0;

	for(int i = 0; i < in_Cluster1.Width; ++i)
	{
		int l_Index1 = in_Cluster1.Length + (in_Cluster1.Length * i) - 1;
		int l_Index2 = in_Cluster2.Length * i;
		if(in_Cluster1.BaseNodes[l_Index1]->Height 
			|| in_Cluster2.BaseNodes[l_Index2]->Height)
		{
			++j;
		}
		else
		{
			l_PotentialGates[j].push_back(std::make_pair(
				std::make_shared<Node>(Node(in_Cluster1.Level, 0, in_Cluster1.BaseNodes[l_Index1]->Position)),
				std::make_shared<Node>(Node(in_Cluster2.Level, 0, in_Cluster2.BaseNodes[l_Index2]->Position))));
		}
	}

	for(int i = 0; i <= j; ++i)
	{
		if(l_PotentialGates[i].size() == m_MaxEntranceWidth && l_PotentialGates[i].size() > 0)
		{
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()/2]);
			in_Cluster1.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()/2].first);
			in_Cluster2.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()/2].second);
		}
		else if(l_PotentialGates[i].size() < m_MaxEntranceWidth && l_PotentialGates[i].size() > 0)
		{
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1]);
			in_Cluster1.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1].first);
			in_Cluster2.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1].second);
		}
		else if(l_PotentialGates[i].size() > m_MaxEntranceWidth && l_PotentialGates[i].size() > 0)
		{
			out_Gates.push_back(l_PotentialGates[i][0]);
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1]);
			in_Cluster1.LevelNodes.push_back(l_PotentialGates[i][0].first);
			in_Cluster2.LevelNodes.push_back(l_PotentialGates[i][0].second);
			in_Cluster1.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1].first);
			in_Cluster2.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1].second);
		}
	}
}

void Navigator::BuildTopEntrances(Cluster & in_Cluster1, Cluster & in_Cluster2, std::vector<Gate> & out_Gates)
{
	std::vector<std::vector<Gate>> l_PotentialGates(in_Cluster1.Width);
	int j = 0;
	int l_Index1 = 0;

	for(int i = 0; i < in_Cluster1.Length; ++i)
	{
		l_Index1 = in_Cluster1.Width*(in_Cluster1.Length-1)+i;
		if(in_Cluster1.BaseNodes[in_Cluster1.Width * (in_Cluster1.Length - 1) + i]->Height 
			|| in_Cluster2.BaseNodes[i]->Height)
		{
			++j;
		}
		else
		{
			l_PotentialGates[j].push_back(std::make_pair(
				std::make_shared<Node>(Node(in_Cluster1.Level, 0, in_Cluster1.BaseNodes[l_Index1]->Position)),
				 std::make_shared<Node>(Node(in_Cluster2.Level, 0, in_Cluster2.BaseNodes[i]->Position))));
		}
	}

	for(int i = 0; i <= j; ++i)
	{
		if(l_PotentialGates[i].size() == m_MaxEntranceWidth && l_PotentialGates[i].size() > 0)
		{
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()/2]);
			in_Cluster1.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()/2].first);
			in_Cluster2.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()/2].second);
		}
		else if(l_PotentialGates[i].size() < m_MaxEntranceWidth && l_PotentialGates[i].size() > 0)
		{
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1]);
			in_Cluster1.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1].first);
			in_Cluster2.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1].second);
		}
		else if(l_PotentialGates[i].size() > m_MaxEntranceWidth && l_PotentialGates[i].size() > 0)
		{
			out_Gates.push_back(l_PotentialGates[i][0]);
			out_Gates.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1]);
			in_Cluster1.LevelNodes.push_back(l_PotentialGates[i][0].first);
			in_Cluster2.LevelNodes.push_back(l_PotentialGates[i][0].second);
			in_Cluster1.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1].first);
			in_Cluster2.LevelNodes.push_back(l_PotentialGates[i][l_PotentialGates[i].size()-1].second);
		}
	}
}

void Navigator::BuildGraph()
{
	m_Graphs.push_back(Graph());

	std::for_each(m_Entrances[0].begin(), m_Entrances[0].end(), [this](const Entrance & in_Entrance)
	{
		for(auto l_GatesIt = in_Entrance.Gates.begin(); l_GatesIt != in_Entrance.Gates.end(); ++l_GatesIt)
		{
			// Gate transitions always cost 1
			m_Graphs[1][l_GatesIt->first][l_GatesIt->second] = 1.0;
			m_Graphs[1][l_GatesIt->second][l_GatesIt->first] = 1.0;
		}
	});

	std::for_each(m_Clusters[1].begin(), m_Clusters[1].end(), [this](Cluster & in_Cluster)
	{
		BuildLocalGraph(in_Cluster.Length, in_Cluster.Width, in_Cluster.BaseNodes, in_Cluster.LocalGraph);
	});

	ConnectLevelNodes();

	// Setting the iterators to make the Navigator interruptible
	m_CurrentCluster = m_Clusters[1].begin();
	m_CurrentFirstNode = m_CurrentCluster->BaseNodes.begin();
	m_CurrentSecondNode = m_CurrentCluster->BaseNodes.begin() + 1;
	m_NodesEnd = m_CurrentCluster->BaseNodes.end();
}

// TODO : Generalize to multiple layers ??
void Navigator::ConnectLevelNodes()
{
	for(m_CurrentCluster = m_Clusters[1].begin(); m_CurrentCluster != m_Clusters[1].end(); ++m_CurrentCluster)
	{
		for(unsigned i = 0; i < m_CurrentCluster->LevelNodes.size()-1; ++i)
		{
			m_CurrentFirstNode = FindCorrespondingBaseNode(*m_CurrentCluster, m_CurrentCluster->LevelNodes[i]);
			for(unsigned j = i + 1; j < m_CurrentCluster->LevelNodes.size(); ++j)
			{
				m_CurrentSecondNode = FindCorrespondingBaseNode(*m_CurrentCluster, m_CurrentCluster->LevelNodes[j]);
				m_NodesEnd = m_CurrentCluster->LevelNodes.end();
		
				double l_Distance = AStar(*m_CurrentFirstNode, *m_CurrentSecondNode, m_CurrentCluster->LocalGraph, TrivialHeuristic());

				if(l_Distance < std::numeric_limits<double>::infinity())
				{
					m_Graphs[1][m_CurrentCluster->LevelNodes[i]][m_CurrentCluster->LevelNodes[j]] = l_Distance;
					m_Graphs[1][m_CurrentCluster->LevelNodes[j]][m_CurrentCluster->LevelNodes[i]] = l_Distance;
				}
			}
		}		
	}
}

Navigator::NodeIterator Navigator::FindCorrespondingBaseNode(Cluster & in_Cluster, const std::shared_ptr<Navigator::Node> & in_LevelNode) const
{
	return std::find_if(in_Cluster.BaseNodes.begin(), in_Cluster.BaseNodes.end(), 
				[&in_Cluster, &in_LevelNode](const std::shared_ptr<Node> & in_Node)
			{
				return in_Node->Height == in_LevelNode->Height
					&& in_Node->Position == in_LevelNode->Position;

			});
}

Navigator::NodeIterator Navigator::FindCorrespondingBaseNode(Cluster & in_Cluster, const Navigator::Node & in_LevelNode) const
{
	return std::find_if(in_Cluster.BaseNodes.begin(), in_Cluster.BaseNodes.end(), 
				[&in_Cluster, &in_LevelNode](const std::shared_ptr<Node> & in_Node)
			{
				return in_Node->Height == in_LevelNode.Height
					&& in_Node->Position == in_LevelNode.Position;

			});
}

void Navigator::AddIntraEdges(const double in_Time)
{
	boost::chrono::high_resolution_clock::time_point l_Start = boost::chrono::high_resolution_clock::now();

	ResumableEmbeddedLoop(m_CurrentFirstNode, m_CurrentSecondNode, m_NodesEnd, 
		[this]()
		{
			double l_Distance = AStar(*m_CurrentFirstNode, *m_CurrentSecondNode, m_CurrentCluster->LocalGraph, TrivialHeuristic());

			if(l_Distance < std::numeric_limits<double>::infinity())
			{
				m_CurrentCluster->LocalGraph[*m_CurrentFirstNode][*m_CurrentSecondNode] = l_Distance;
			}
		}, 
		[&l_Start, &in_Time]()
		{ return in_Time - (boost::chrono::high_resolution_clock::now() - l_Start).count() > 0.0; });
}

void Navigator::Preprocess()
{
	AbstractMaze();
	BuildGraph();
}

// ********** Online processing **********

void Navigator::ConnectToBorder(const std::shared_ptr<Node> & in_Node, Cluster & in_Cluster)
{
	for(auto l_It = in_Cluster.LevelNodes.begin(); l_It != in_Cluster.LevelNodes.end(); ++l_It)
	{
		double l_Distance = AStar(*FindCorrespondingBaseNode(in_Cluster, in_Node), *FindCorrespondingBaseNode(in_Cluster, *l_It), 
			in_Cluster.LocalGraph, TrivialHeuristic());

		if(l_Distance < std::numeric_limits<double>::infinity())
		{
			m_Graphs[in_Cluster.Level][in_Node][*l_It] = l_Distance;
			m_Graphs[in_Cluster.Level][*l_It][in_Node] = l_Distance;
		}
	}
}

void Navigator::InsertNode(const std::shared_ptr<Node> & in_Node, const int in_Level)
{
	for(int i = 1; i <= in_Level; ++i)
	{
		auto l_ClusterIt = m_Clusters[in_Level].begin();
		for(; l_ClusterIt != m_Clusters[in_Level].end(); ++l_ClusterIt)
		{
			auto l_MinMax = std::minmax_element(l_ClusterIt->BaseNodes.begin(), l_ClusterIt->BaseNodes.end(), NodeComparer());
			if((*l_MinMax.first)->Position.x <= in_Node->Position.x && in_Node->Position.x <= ((*l_MinMax.second)->Position.x)
				&& (*l_MinMax.first)->Position.y <= in_Node->Position.y && in_Node->Position.y <= ((*l_MinMax.second)->Position.y))
				break;
		}
		ConnectToBorder(in_Node, *l_ClusterIt);
		l_ClusterIt->LevelNodes.push_back(in_Node);
	}
}

std::vector<Navigator::Node> Navigator::ComputeAbstractPath(const Vector2 & in_Start, const Vector2 & in_Goal)
{
	std::shared_ptr<Node> l_StartNode;
	std::shared_ptr<Node> l_EndNode;
	Vector2 l_StartPos(floor(in_Start.x + 0.5f), floor(in_Start.y + 0.5f));
	Vector2 l_GoalPos(floor(in_Goal.x + 0.5f), floor(in_Goal.y + 0.5f));

	auto l_StartIt = m_Clusters[1].begin();
	for(; l_StartIt != m_Clusters[1].end(); ++l_StartIt)
	{
		auto l_NodesIt = l_StartIt->LevelNodes.begin();
		for(; l_NodesIt != l_StartIt->LevelNodes.end(); ++l_NodesIt)
		{
			if((*l_NodesIt)->Height == 0 && (*l_NodesIt)->Level == 1 && (*l_NodesIt)->Position == l_StartPos)
			{
				l_StartNode = *l_NodesIt;
				break;
			}
		}
		if(l_NodesIt != l_StartIt->LevelNodes.end())
			break;
	}

	auto l_GoalIt = m_Clusters[1].begin();
	for(; l_GoalIt != m_Clusters[1].end(); ++l_GoalIt)
	{
		auto l_NodesIt = l_GoalIt->LevelNodes.begin();
		for(; l_NodesIt != l_GoalIt->LevelNodes.end(); ++l_NodesIt)
		{
			if((*l_NodesIt)->Height == 0 && (*l_NodesIt)->Level == 1 && (*l_NodesIt)->Position == l_GoalPos)
			{
				l_EndNode = *l_NodesIt;
				break;
			}
		}
		if(l_NodesIt != l_GoalIt->LevelNodes.end())
			break;
	}

	if(l_StartIt == m_Clusters[1].end())
		l_StartNode = std::make_shared<Node>(Node(1, 0, l_StartPos));

	if(l_GoalIt == m_Clusters[1].end())
		l_EndNode = std::make_shared<Node>(Node(1, 0, l_GoalPos));

	std::vector<Node> l_AbstractPath;
	
	if(m_Paths[l_StartNode][l_EndNode] != std::vector<std::shared_ptr<Node>>())
	{
		std::for_each(m_Paths[l_StartNode][l_EndNode].begin(), m_Paths[l_StartNode][l_EndNode].end(), 
			[&l_AbstractPath](const std::shared_ptr<Node> & in_Node)
		{
			l_AbstractPath.push_back(*in_Node);
		});
		return l_AbstractPath;
	}

	InsertNode(l_StartNode, 1);
	InsertNode(l_EndNode, 1);
	
	AStar(l_StartNode, l_EndNode, m_Graphs[1], TrivialHeuristic());

	std::for_each(m_Paths[l_StartNode][l_EndNode].begin(), m_Paths[l_StartNode][l_EndNode].end(), 
		[&l_AbstractPath](const std::shared_ptr<Node> & in_Node)
	{
		l_AbstractPath.push_back(*in_Node);
	});
	return l_AbstractPath;
}

std::vector<Vector2> Navigator::ComputeConcretePath(const Node & in_StartNode, const Node & in_GoalNode)
{
	// Find the cluster which they belong to
	auto l_ClusterIt = std::find_if(m_Clusters[1].begin(), m_Clusters[1].end(),
		[&in_StartNode, &in_GoalNode](const Cluster & in_Cluster) -> bool
	{
		int found = 0;
		for(auto l_It = in_Cluster.LevelNodes.begin(); l_It != in_Cluster.LevelNodes.end(); ++l_It)
		{
			if(**l_It == in_StartNode)
			{
				if(++found == 2)
					break;
			}
			else if(**l_It == in_GoalNode)
			{
				if(++found == 2)
					break;
			}
		}
		return found == 2;
	});

	if(l_ClusterIt == m_Clusters[1].end())
	{
		return std::vector<Vector2>();
	}

	std::shared_ptr<Navigator::Node> l_BaseStart(*FindCorrespondingBaseNode(*l_ClusterIt, in_StartNode));
	std::shared_ptr<Navigator::Node> l_BaseGoal(*FindCorrespondingBaseNode(*l_ClusterIt, in_GoalNode));

	if(m_Paths[l_BaseStart][l_BaseGoal] == std::vector<std::shared_ptr<Node>>())
		AStar(l_BaseStart, l_BaseGoal, l_ClusterIt->LocalGraph, TrivialHeuristic());

	std::vector<Vector2> l_ConcretePath;
	std::for_each(m_Paths[l_BaseStart][l_BaseGoal].begin(), m_Paths[l_BaseStart][l_BaseGoal].end(), 
		[&l_ConcretePath](const std::shared_ptr<Node> & in_Node)
	{
		l_ConcretePath.push_back(in_Node->Position);
	});
	return l_ConcretePath;
}

void Navigator::ProcessClusters(const double in_Time)
{
	boost::chrono::high_resolution_clock::time_point l_Start = boost::chrono::high_resolution_clock::now();

	ResumableForEach(m_CurrentCluster, m_Clusters[1].end(), 
		[&in_Time, this]() { AddIntraEdges(in_Time); }, 
		[&in_Time, &l_Start]() { return in_Time - (boost::chrono::high_resolution_clock::now() - l_Start).count() > 0.0; });
}
