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
	m_Clusters.push_back(std::vector<Cluster>(1, Cluster(0, in_Length, in_Width, NodeVector(), NodeVector(in_Length * in_Width))));
	auto l_Cluster = m_Clusters[0].begin();
	std::for_each(in_Level.get(), in_Level.get() + in_Length * in_Width, [&l_Cluster, &in_Width, &l_X, &l_Y, this](const float in_Block)
	{
		// The first level cluster is in fact the whole map
		l_Cluster->LevelNodes[l_X + l_Y * in_Width] = (std::make_shared<Node>(Navigator::Node(0, static_cast<int>(in_Block), Vector2(static_cast<float>(l_X), static_cast<float>(l_Y)))));
		
		if(l_X < in_Width - 1)
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
						Graph & in_Graph, IHeuristic && in_Heuristic)
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
			NodeVector l_Path;
			
			// Reverse the parents' chain to get the path
			std::shared_ptr<Node> l_ParentNode = l_Parents[l_CurrentNode];

			l_Path.push_back(l_CurrentNode);
			 
			while(l_ParentNode != in_Start)
			{
				l_Path.push_back(l_ParentNode);
				l_ParentNode = l_Parents[l_ParentNode];
			}
			l_Path.push_back(in_Start);

			m_Paths[in_Goal][in_Start] = l_Path;
			std::reverse(l_Path.begin(), l_Path.end());
			m_Paths[in_Start][in_Goal] = l_Path;

			return l_RealCosts[l_CurrentNode];
		}

		if(in_Graph[l_CurrentNode] != std::map<std::shared_ptr<Node>, double, NodeComparer>())
		{
			for(auto l_NeighborIt = in_Graph.at(l_CurrentNode).begin(); l_NeighborIt != in_Graph.at(l_CurrentNode).end(); ++l_NeighborIt)
			{
				double l_TentativeRealCost = l_RealCosts[l_CurrentNode] + l_NeighborIt->second;

				auto l_ClosedNodeIt = std::find_if(l_Closed.begin(), l_Closed.end(), 
					[&l_NeighborIt](const std::pair<double, std::shared_ptr<Node>> & in_Node)
				{
					return *in_Node.second == *l_NeighborIt->first;
				});

				double l_HeuristicCost = in_Heuristic(*l_NeighborIt->first, *in_Goal);
				if(l_ClosedNodeIt != l_Closed.end())
				{
					if(l_TentativeRealCost + l_HeuristicCost <= l_NeighborIt->second)
					{
						l_Opened.insert(*l_ClosedNodeIt);
						l_Closed.erase(l_ClosedNodeIt);
					}
					continue;
				}

				auto l_OpenedIt = std::find_if(l_Opened.begin(), l_Opened.end(), 
					[&l_NeighborIt](const std::pair<double, std::shared_ptr<Node>> & in_Node)
				{
					return *in_Node.second == *l_NeighborIt->first;
				});

				if(l_OpenedIt != l_Opened.end())
				{
					if(l_TentativeRealCost + l_HeuristicCost <= l_NeighborIt->second)
					{
						l_Opened.insert(*l_OpenedIt);
						l_Closed.erase(l_OpenedIt);
					}
					continue;
				}

				if(l_OpenedIt == l_Opened.end() || l_TentativeRealCost < l_RealCosts[l_NeighborIt->first])
				{
					l_Parents[l_NeighborIt->first] = l_CurrentNode;
					l_RealCosts[l_NeighborIt->first] = l_TentativeRealCost;
					if(l_OpenedIt == l_Opened.end())
					{
						l_Opened.insert(std::make_pair(l_RealCosts[l_NeighborIt->first] + l_HeuristicCost, l_NeighborIt->first));
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
	if(in_Cluster1.MinPos.x == in_Cluster2.MaxPos.x+1 && in_Cluster1.MinPos.y == in_Cluster2.MinPos.y)
	{
		l_Ok = true;
		out_Adjacency = Right;
	}
	// Cluster1 below Cluster2 ?
	else if(in_Cluster1.MinPos.y == in_Cluster2.MaxPos.y+1 && in_Cluster1.MinPos.x == in_Cluster2.MinPos.x)
	{
		l_Ok = true;
		out_Adjacency = Below;
	}
	// Cluster1 to left of Cluster2 ?
	else if(in_Cluster1.MaxPos.x == in_Cluster2.MinPos.x-1 && in_Cluster1.MinPos.y == in_Cluster2.MinPos.y)
	{
		l_Ok = true;
		out_Adjacency = Left;
	}
	// Cluster1 above Cluster2 ?
	else if(in_Cluster1.MaxPos.y == in_Cluster2.MinPos.y-1 && in_Cluster1.MinPos.x == in_Cluster2.MinPos.x)
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

void Navigator::BuildLocalGraph(const int in_Length, const int in_Width, const Navigator::NodeVector & in_Nodes, Graph & out_LocalGraph)
{
	int l_CurrentNodeIndex = 0;	
	int l_RightNodeIndex = 0;
	int l_DownRightIndex = 0;
	int l_DownIndex = 0;
	int l_DownLeftIndex = 0;

	for(int i  = 0; i < in_Length; ++i)
	{
		for(int j = 0; j < in_Width; ++j)
		{
			l_CurrentNodeIndex = j + i * in_Width;
			l_RightNodeIndex = l_CurrentNodeIndex+1;
			l_DownRightIndex = (j+1) + (i+1) * in_Width;
			l_DownIndex = j + (i+1) * in_Width;
			l_DownLeftIndex = (j-1) + (i+1) * in_Width;
			if(!in_Nodes[l_CurrentNodeIndex]->Height)
			{
				// Link the right node
				if(j < in_Width-1 && !in_Nodes[l_CurrentNodeIndex+1]->Height)
				{
					out_LocalGraph[in_Nodes[l_CurrentNodeIndex]][in_Nodes[l_RightNodeIndex]] = 1.0;
					out_LocalGraph[in_Nodes[l_RightNodeIndex]][in_Nodes[l_CurrentNodeIndex]] = 1.0;
				}
				// Link the down-right node
				if(j < in_Width-1 && i < in_Length-1 && !in_Nodes[l_DownRightIndex]->Height)
				{
					out_LocalGraph[in_Nodes[l_CurrentNodeIndex]][in_Nodes[l_DownRightIndex]] = 1.42;
					out_LocalGraph[in_Nodes[l_DownRightIndex]][in_Nodes[l_CurrentNodeIndex]] = 1.42;
				}
				// Link the down node
				if(i < in_Length-1 && !in_Nodes[l_DownIndex]->Height)
				{
					out_LocalGraph[in_Nodes[l_CurrentNodeIndex]][in_Nodes[l_DownIndex]] = 1.0;
					out_LocalGraph[in_Nodes[l_DownIndex]][in_Nodes[l_CurrentNodeIndex]] = 1.0;
				}
				// Link the down-left node
				if(j > 0 && i < in_Length-1 && !in_Nodes[l_DownLeftIndex]->Height)
				{
					out_LocalGraph[in_Nodes[l_CurrentNodeIndex]][in_Nodes[l_DownLeftIndex]] = 1.42;
					out_LocalGraph[in_Nodes[l_DownLeftIndex]][in_Nodes[l_CurrentNodeIndex]] = 1.42;
				}
			}
		}
	}
}

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
		for(int i = l_LengthOffset; i < l_ClusterLength + l_LengthOffset; ++i)
		{
			for(int j = l_WidthOffset; j < l_ClusterWidth + l_WidthOffset; ++j)
			{
				l_It->BaseNodes.push_back(m_Clusters[in_Level - 1].begin()->LevelNodes[j + (i * l_Width)]);
			}
		}
		if(l_WidthOffset + l_ClusterWidth < l_Width)
		{
			l_WidthOffset += l_ClusterWidth;	
		}
		else
		{
			l_WidthOffset = 0;
			l_LengthOffset += l_ClusterLength;
		}
		auto l_MinMax = std::minmax_element(l_It->BaseNodes.begin(), l_It->BaseNodes.end(), PositionComparer());
		l_It->MinPos = (*l_MinMax.first)->Position;
		l_It->MaxPos = (*l_MinMax.second)->Position;
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
	// Removing duplicates in LevelNodes
	std::sort(in_Cluster1.LevelNodes.begin(), in_Cluster1.LevelNodes.end());
	std::sort(in_Cluster2.LevelNodes.begin(), in_Cluster2.LevelNodes.end());
	in_Cluster1.LevelNodes.erase(std::unique(in_Cluster1.LevelNodes.begin(), in_Cluster1.LevelNodes.end()), in_Cluster1.LevelNodes.end());
	in_Cluster2.LevelNodes.erase(std::unique(in_Cluster2.LevelNodes.begin(), in_Cluster2.LevelNodes.end()), in_Cluster2.LevelNodes.end());
}

void Navigator::BuildSideEntrances(Cluster & in_Cluster1, Cluster & in_Cluster2, std::vector<Gate> & out_Gates)
{
	std::vector<std::vector<Gate>> l_PotentialGates(in_Cluster1.Length);
	int j = 0;

	for(int i = 0; i < in_Cluster1.Length; ++i)
	{
		int l_Index1 = (i+1) * in_Cluster1.Width - 1;
		int l_Index2 = in_Cluster2.Width * i;
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

	for(int i = 0; i < in_Cluster1.Width; ++i)
	{
		l_Index1 = in_Cluster1.Width*(in_Cluster1.Length-1)+i;
		if(in_Cluster1.BaseNodes[l_Index1]->Height 
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

	// Setting the iterators to make the Navigator resumable
	m_CurrentCluster = m_Clusters[1].begin();
	m_CurrentFirstNode = m_CurrentCluster->BaseNodes.begin();
	m_CurrentSecondNode = m_CurrentCluster->BaseNodes.begin() + 1;
	m_NodesEnd = m_CurrentCluster->BaseNodes.end();
}

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
				return in_Node->Position == in_LevelNode->Position;
			});
}

Navigator::NodeIterator Navigator::FindCorrespondingBaseNode(Cluster & in_Cluster, const Navigator::Node & in_LevelNode) const
{
	return std::find_if(in_Cluster.BaseNodes.begin(), in_Cluster.BaseNodes.end(), 
				[&in_Cluster, &in_LevelNode](const std::shared_ptr<Node> & in_Node)
			{
				return in_Node->Position == in_LevelNode.Position;
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

Navigator::NodeVector Navigator::ComputeAbstractPath(const Vector2 & in_Start, const Vector2 & in_Goal)
{
	if(in_Start == in_Goal)
		return std::vector<std::shared_ptr<Node>>();

	std::shared_ptr<Node> l_StartNode;
	std::shared_ptr<Node> l_EndNode;
	auto l_CIt = m_Clusters[0].begin();

	Vector2 l_StartPos(in_Start.x >= l_CIt->Width ? l_CIt->Width : floor(in_Start.x + 0.5f), 
		in_Start.y >= l_CIt->Length ? l_CIt->Length : floor(in_Start.y + 0.5f));
	Vector2 l_GoalPos(in_Goal.x >= l_CIt->Width ? l_CIt->Width : floor(in_Goal.x + 0.5f), 
		in_Goal.y >= l_CIt->Length ? l_CIt->Length : floor(in_Goal.y + 0.5f));

	auto l_GoalIt = m_Clusters[1].begin();
	for(; l_GoalIt != m_Clusters[1].end(); ++l_GoalIt)
	{
		if(l_GoalIt->MinPos <= l_GoalPos && l_GoalPos <= l_GoalIt->MaxPos)
			break;
	}
	
	if(l_GoalIt == m_Clusters[1].end())
		return std::vector<std::shared_ptr<Node>>();
	
	for(auto l_NodesIt = l_GoalIt->LevelNodes.begin(); l_NodesIt != l_GoalIt->LevelNodes.end(); ++l_NodesIt)
	{
		if((*l_NodesIt)->Height == 0 && (*l_NodesIt)->Level == 1 && (*l_NodesIt)->Position == l_GoalPos)
		{
			l_EndNode = *l_NodesIt;
			break;
		}
	}

	auto l_StartIt = m_Clusters[1].begin();
	for(; l_StartIt != m_Clusters[1].end(); ++l_StartIt)
	{
		if(l_StartIt->MinPos <= l_StartPos && l_StartPos <= l_StartIt->MaxPos)
			break;
	}
	
	if(l_StartIt == m_Clusters[1].end())
		return std::vector<std::shared_ptr<Node>>();
	
	for(auto l_NodesIt = l_StartIt->LevelNodes.begin(); l_NodesIt != l_StartIt->LevelNodes.end(); ++l_NodesIt)
	{
		if((*l_NodesIt)->Height == 0 && (*l_NodesIt)->Level == 1 && (*l_NodesIt)->Position == l_StartPos)
		{
			l_StartNode = *l_NodesIt;
			break;
		}
	}

	if(!l_StartNode)
	{
		l_StartNode = std::make_shared<Node>(Node(1, 0, l_StartPos));
		l_StartIt->LevelNodes.push_back(l_StartNode);
	}

	if(!l_EndNode)
	{
		l_EndNode = std::make_shared<Node>(Node(1, 0, l_GoalPos));
		l_GoalIt->LevelNodes.push_back(l_EndNode);
	}

	if(m_Paths[l_StartNode][l_EndNode] != NodeVector())
		return NodeVector(m_Paths[l_StartNode][l_EndNode].begin(), m_Paths[l_StartNode][l_EndNode].end());

	ConnectToBorder(l_StartNode, *l_StartIt);
	ConnectToBorder(l_EndNode, *l_GoalIt);
	
	AStar(l_StartNode, l_EndNode, m_Graphs[1], ManhattanDistance());

	return NodeVector(m_Paths[l_StartNode][l_EndNode].begin(), m_Paths[l_StartNode][l_EndNode].end());
}

std::vector<Vector2> Navigator::ComputeConcretePath(const std::shared_ptr<Node> & in_StartNode, const std::shared_ptr<Node> & in_GoalNode)
{
	if(!in_StartNode || !in_GoalNode)
		return std::vector<Vector2>();

	// Find the cluster which they belong to
	auto l_ClusterIt = m_Clusters[1].begin();
	for(; l_ClusterIt != m_Clusters[1].end(); ++l_ClusterIt)
		if(l_ClusterIt->MinPos <= in_StartNode->Position && in_StartNode->Position <= l_ClusterIt->MaxPos
			&& l_ClusterIt->MinPos <= in_GoalNode->Position && in_GoalNode->Position <= l_ClusterIt->MaxPos)
			break;

	if(l_ClusterIt == m_Clusters[1].end())
		return std::vector<Vector2>();

	std::shared_ptr<Navigator::Node> l_BaseStart(*FindCorrespondingBaseNode(*l_ClusterIt, in_StartNode));
	std::shared_ptr<Navigator::Node> l_BaseGoal(*FindCorrespondingBaseNode(*l_ClusterIt, in_GoalNode));
	
	if(m_Paths[l_BaseStart][l_BaseGoal] == NodeVector())
		AStar(l_BaseStart, l_BaseGoal, l_ClusterIt->LocalGraph, TrivialHeuristic());

	std::vector<Vector2> l_ConcretePath;
	std::transform(m_Paths[l_BaseStart][l_BaseGoal].begin(), m_Paths[l_BaseStart][l_BaseGoal].end(), std::back_inserter(l_ConcretePath),
		[](const std::shared_ptr<Node> & in_Node)
		{
			return in_Node->Position;
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
