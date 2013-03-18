#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <memory>
#include <vector>
#include <utility>

#include "api\Vector2.h"

class IHeuristic;

/*
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
		
		Node(const int in_Level = 0, const int in_Height = 0, const Vector2 & in_Position = Vector2()) : 
			Level(in_Level), Height(in_Height), Position(in_Position) { }

		Node(const Node & in_Node) : Level(in_Node.Level), Height(in_Node.Height), Position(in_Node.Position) { }

		Node(Node && in_Node) : Level(std::move(in_Node.Level)), Height(std::move(in_Node.Height)), Position(std::move(in_Node.Position)) { }

		bool operator==(const Node & in_Node) const
		{
			return Level == in_Node.Level && Height == in_Node.Height && Position == in_Node.Position;
		}
	};

private:
	enum EdgeType { Intra, Inter };
	enum Adjacency { Above, Below, Left, Right };

	struct Edge
	{
		int Level;
		double Weight;
		EdgeType Type;
		std::pair<Node, Node> Nodes;

		Edge(const Node & in_Node1 = Node(), const Node & in_Node2 = Node(), const int in_Level = 0, 
			const double in_Weight = 0.f, EdgeType in_Type = Intra) : 
		Nodes(std::make_pair(in_Node1, in_Node2)), Level(in_Level), Weight(in_Weight), Type(in_Type) { }

		Edge(Edge && in_Edge) : Level(std::move(in_Edge.Level)), Weight(in_Edge.Weight), Type(in_Edge.Type), Nodes(in_Edge.Nodes) { }
	};

	struct Cluster
	{
		int Level;
		int Length;
		int Width;
		std::vector<Node> Nodes;
		std::vector<Edge> Edges;

		Cluster(const int in_Level = 0, const int in_Length = 1, const int in_Width = 1, 
			const std::vector<Node> & in_Nodes = std::vector<Node>(), const std::vector<Edge> & in_Edges = std::vector<Edge>()) :
		Level(in_Level), Length(in_Length), Width(in_Width), Nodes(in_Nodes), Edges(in_Edges) { }

		Cluster(Cluster && in_Cluster) : Level(std::move(in_Cluster.Level)), Length(std::move(in_Cluster.Length)),
			Width(std::move(in_Cluster.Width)),	Nodes(std::move(in_Cluster.Nodes)), Edges(std::move(in_Cluster.Edges)) { }
	};

	struct Entrance
	{
		std::pair<Cluster, Cluster> Clusters;
		std::vector<std::pair<Node, Node>> Gates;

		Entrance(const Cluster & in_Cluster1, const Cluster & in_Cluster2, const std::vector<std::pair<Node, Node>> in_Gates) :
			Clusters(std::make_pair<Cluster, Cluster>(in_Cluster1, in_Cluster2)), Gates(in_Gates) { }

		Entrance(Entrance && in_Entrance) : Clusters(std::move(in_Entrance.Clusters)), Gates(std::move(in_Entrance.Gates)) {}
	};

	struct Comparator
	{
		bool operator()(const std::pair<Node, double> in_Value1, const std::pair<Node, double> in_Value2) const
		{
			return in_Value1.second < in_Value2.second;
		}
	};

private:
	int m_MaxEntranceWidth;
	std::vector<std::vector<Cluster>> m_Clusters;
	std::vector<std::vector<Entrance>> m_Entrances;
	std::vector<std::vector<Node>> m_Nodes;
	std::vector<std::vector<Edge>> m_Edges;

public:
	Navigator(const std::unique_ptr<float[]> & in_Level, const int in_Length, const int in_Width, const int in_MaxEntranceWidth = 3);

	Vector2 GetBestDirection(const Vector2 & in_Start, const Vector2 & in_Goal, const int in_Level);

private:
	double AStar(const Node & in_Start, const Node & in_Goal, const int in_Level, const IHeuristic & in_Heuristic) const;

	// Offline processing

	void AbstractMaze();
	bool Adjacent(const Cluster & in_Cluster1, const Cluster & in_Cluster2, Adjacency & out_Adjacency) const;
	void BuildClusters(const int in_Level);
	void BuildEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, const int in_Level, const Adjacency in_Adjacency);
	void BuildSideEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, std::vector<std::pair<Node, Node>> & out_Gates);
	void BuildTopEntrances(const Cluster & in_Cluster1, const Cluster & in_Cluster2, std::vector<std::pair<Node, Node>> & out_Gates);
	void BuildGraph();
	void AddIntraEdgesToClusters(const int in_Level);
	void Preprocess();

	// Online processing

	void ConnectToBorder(const Node & in_Node, Cluster & in_Cluster);
	void InsertNode(Node && in_Node, const int in_Level);
	double SearchForDistance(const Node & in_Node1, const Node & in_Node2, const Cluster & in_Cluster) const;
	// HierachichalSearch ?= GetBestDirection

	void UpdateHeuristic();

};

#endif // NAVIGATOR_H
