#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <utility>

#include <boost/chrono/chrono.hpp>

#include "api\Vector2.h"

class IHeuristic;

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* Navigator
* Plan the shortest safe distance toward a goal for the bots by using the HPA* algorithm.
* For now, it only builds one abstract layer.
*/
class Navigator
{
public:
	struct Node
	{
		int Level;
		int Height;
		Vector2 Position;
		
		Node(const int in_Level = 0, const int in_Height = 0, const Vector2 & in_Position = Vector2(0.f, 0.f)) : 
			Level(in_Level), Height(in_Height), Position(in_Position) { }

		Node(const Node & in_Node) : Level(in_Node.Level), Height(in_Node.Height), Position(in_Node.Position) { }

		Node(Node && in_Node) : Level(std::move(in_Node.Level)), Height(std::move(in_Node.Height)), Position(std::move(in_Node.Position)) { }

		bool operator==(const Node & in_Node) const
		{
			return Level == in_Node.Level && Height == in_Node.Height && Position == in_Node.Position;
		}

		bool operator!=(const Node & in_Node) const
		{
			return !(*this == in_Node);
		}

		bool operator<(const Node & in_Node) const
		{
			return Level < in_Node.Level 
				|| (Level == in_Node.Level && Position.y < in_Node.Position.y) 
				|| (Level == in_Node.Level && Position.y == in_Node.Position.y && Position.x < in_Node.Position.x) 
				|| (Level == in_Node.Level && Position == in_Node.Position && Height < in_Node.Height);
		}
	};

private:
	struct NodeComparer
	{
		bool operator()(const std::shared_ptr<Node> & in_Node1, const std::shared_ptr<Node> & in_Node2)
		{
			return *in_Node1 < *in_Node2;
		}
	};

private:
	typedef std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, double, NodeComparer>, NodeComparer> Graph;
	typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> Gate;
	typedef std::vector<std::shared_ptr<Node>>::iterator NodeIterator;
	typedef std::vector<std::shared_ptr<Node>>::const_iterator ConstNodeIterator;

private:
	enum Adjacency { Above, Below, Left, Right };

	struct Cluster
	{
		int Level;
		int Length;
		int Width;
		std::vector<std::shared_ptr<Node>> BaseNodes;
		std::vector<std::shared_ptr<Node>> LevelNodes;
		Graph LocalGraph;

		Cluster(const int in_Level = 0, const int in_Length = 1, const int in_Width = 1, 
			const std::vector<std::shared_ptr<Node>> & in_BaseNodes = std::vector<std::shared_ptr<Node>>(),
			const std::vector<std::shared_ptr<Node>> & in_LevelNodes = std::vector<std::shared_ptr<Node>>()) :
		Level(in_Level), Length(in_Length), Width(in_Width), BaseNodes(in_BaseNodes), LevelNodes(in_LevelNodes) { }

		Cluster(Cluster && in_Cluster) : Level(std::move(in_Cluster.Level)), Length(std::move(in_Cluster.Length)),
			Width(std::move(in_Cluster.Width)), BaseNodes(std::move(in_Cluster.BaseNodes)), 
			LevelNodes(std::move(in_Cluster.LevelNodes)), LocalGraph(std::move(in_Cluster.LocalGraph)) { }

		bool operator==(const Cluster & in_Cluster) const
		{
			return Level == in_Cluster.Level && Length == in_Cluster.Length && Width == in_Cluster.Width
				&& BaseNodes == in_Cluster.BaseNodes && LevelNodes == in_Cluster.LevelNodes && LocalGraph == in_Cluster.LocalGraph;
		}
	};

	struct Entrance
	{
		std::pair<Cluster, Cluster> Clusters;
		std::vector<Gate> Gates;

		Entrance(const Cluster & in_Cluster1, const Cluster & in_Cluster2, 
			const std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> in_Gates) :
			Clusters(std::make_pair<Cluster, Cluster>(in_Cluster1, in_Cluster2)), Gates(in_Gates) { }

		Entrance(Entrance && in_Entrance) : Clusters(std::move(in_Entrance.Clusters)), Gates(std::move(in_Entrance.Gates)) {}
	};

private:
	static const int M_MAXCLUSTERSIZE;
	unsigned m_MaxEntranceWidth;
	std::vector<std::vector<Cluster>> m_Clusters;
	std::vector<std::vector<Entrance>> m_Entrances;
	std::vector<Graph> m_Graphs;
	std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>>> m_Paths;

	// Iterators to make the Navigator interruptible
	std::vector<Cluster>::iterator m_CurrentCluster;
	NodeIterator m_CurrentFirstNode;
	NodeIterator m_CurrentSecondNode;
	NodeIterator m_NodesEnd;

public:
	Navigator() { }
	void Init(const std::unique_ptr<float[]> & in_Level, const int in_Length, const int in_Width, const int in_MaxEntranceWidth = 3);
	void Reset();

	std::vector<Node> ComputeAbstractPath(const Vector2 & in_Start, const Vector2 & in_Goal);
	std::vector<Vector2> ComputeConcretePath(const Node & in_StartNode, const Node & in_GoalNode);
	void ProcessClusters(const double in_Time);

private:
	double AStar(const std::shared_ptr<Node> & in_Start, const std::shared_ptr<Node> & in_Goal, 
		Graph & in_Graph, const IHeuristic & in_Heuristic);

	void AbstractMaze();
	bool Adjacent(const Cluster & in_Cluster1, const Cluster & in_Cluster2, Adjacency & out_Adjacency) const;
	void BuildClusters(const int in_Level);
	int ClusterSize(const int in_Size) const;
	void BuildEntrances(Cluster & in_Cluster1, Cluster & in_Cluster2, const int in_Level, const Adjacency in_Adjacency);
	void BuildSideEntrances(Cluster & in_Cluster1, Cluster & in_Cluster2, std::vector<Gate> & out_Gates);
	void BuildTopEntrances(Cluster & in_Cluster1, Cluster & in_Cluster2, std::vector<Gate> & out_Gates);
	void BuildGraph();
	void Preprocess();
	void ConnectLevelNodes();
	void BuildLocalGraph(const int in_Length, const int in_Width, const std::vector<std::shared_ptr<Node>> & in_Nodes, Graph & in_LocalGraph);
	
	void AddIntraEdges(const double in_TimeLimit);
	
	void ConnectToBorder(const std::shared_ptr<Node> & in_Node, Cluster & in_Cluster);
	void InsertNode(const std::shared_ptr<Node> & in_Node, const int in_Level);
	double SearchForDistance(const std::shared_ptr<Node> & in_Node1, const std::shared_ptr<Node> & in_Node2, const Cluster & in_Cluster);

	NodeIterator FindCorrespondingBaseNode(Cluster & in_Cluster, const std::shared_ptr<Node> & in_LevelNode) const;
	NodeIterator FindCorrespondingBaseNode(Cluster & in_Cluster, const Node & in_LevelNode) const;
};

#endif // NAVIGATOR_H
