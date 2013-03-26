#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <map>
#include <memory>
#include <vector>
#include <utility>

#include "api\Vector2.h"

class IHeuristic;

/*
* Author : Felix-Antoine Ouellet
* CIP :	   09 137 551
*
* Navigator
* Plan the shortest safe distance toward a goal for the bots.
* To do so, it uses the HPA* algorithm.
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
			return Level < in_Node.Level ? true : Level == in_Node.Level ? Position < in_Node.Position : false;
		}
	};

private:
	enum Adjacency { Above, Below, Left, Right };

	struct Cluster
	{
		int Level;
		int Length;
		int Width;
		std::vector<std::shared_ptr<Node>> Nodes;
		std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, double>> LocalGraph;

		Cluster(const int in_Level = 0, const int in_Length = 1, const int in_Width = 1, 
			const std::vector<std::shared_ptr<Node>> & in_Nodes = std::vector<std::shared_ptr<Node>>()) :
		Level(in_Level), Length(in_Length), Width(in_Width), Nodes(in_Nodes) { }

		Cluster(Cluster && in_Cluster) : Level(std::move(in_Cluster.Level)), Length(std::move(in_Cluster.Length)),
			Width(std::move(in_Cluster.Width)), Nodes(std::move(in_Cluster.Nodes)), LocalGraph(std::move(in_Cluster.LocalGraph)) { }

		bool operator==(const Cluster & in_Cluster) const
		{
			return Level == in_Cluster.Level && Length == in_Cluster.Length && Width == in_Cluster.Width
				&& Nodes == in_Cluster.Nodes && LocalGraph == in_Cluster.LocalGraph;
		}
	};

	struct Entrance
	{
		std::pair<Cluster, Cluster> Clusters;
		std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> Gates;

		Entrance(const Cluster & in_Cluster1, const Cluster & in_Cluster2, 
			const std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> in_Gates) :
			Clusters(std::make_pair<Cluster, Cluster>(in_Cluster1, in_Cluster2)), Gates(in_Gates) { }

		Entrance(Entrance && in_Entrance) : Clusters(std::move(in_Entrance.Clusters)), Gates(std::move(in_Entrance.Gates)) {}
	};

	struct Comparator
	{
		bool operator()(const std::pair<std::shared_ptr<Node>, double> in_Value1, const std::pair<std::shared_ptr<Node>, double> in_Value2) const
		{
			return in_Value1.second < in_Value2.second;
		}
	};

private:
	static const int M_MAXCLUSTERSIZE;
	unsigned m_MaxEntranceWidth;
	std::vector<std::vector<Cluster>> m_Clusters;
	std::vector<std::vector<Entrance>> m_Entrances;
	std::vector<std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, double>>> m_Graphs;
	std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>>> m_Paths;

public:
	Navigator() { }
	void Init(const std::unique_ptr<float[]> & in_Level, const int in_Length, const int in_Width, const int in_MaxEntranceWidth = 3);

	Vector2 GetBestDirection(const Vector2 & in_Start, const Vector2 & in_Goal);

private:
	double AStar(const std::shared_ptr<Node> & in_Start, const std::shared_ptr<Node> & in_Goal, const int in_Level, const IHeuristic & in_Heuristic);

	// Offline processing

	void AbstractMaze();
	bool Adjacent(const Cluster & in_Cluster1, const Cluster & in_Cluster2, Adjacency & out_Adjacency) const;
	void BuildClusters(const int in_Level);
	int ClusterSize(const int in_Size) const;
	void BuildEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, const int in_Level, const Adjacency in_Adjacency);
	void BuildSideEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, 
		std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> & out_Gates);
	void BuildTopEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, 
		std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>>> & out_Gates);
	void BuildGraph();
	void AddIntraEdgesToClusters(const int in_Level);
	void Preprocess();

	// Online processing

	void ConnectToBorder(const std::shared_ptr<Node> & in_Node, Cluster & in_Cluster);
	void InsertNode(const std::shared_ptr<Node> & in_Node, const int in_Level);
	double SearchForDistance(const std::shared_ptr<Node> & in_Node1, const std::shared_ptr<Node> & in_Node2, const Cluster & in_Cluster);

};

#endif // NAVIGATOR_H
