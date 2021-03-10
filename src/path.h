#pragma once

// A* (like) path-finder.

#include "point.h"
#include "slabpool.h"
#include <map>
#include <list>
#include <vector>

struct Path {
	static inline std::list<Path*> jobs;
	static void tick();

	struct Node {
		Point point = {0,0,0};
		double gScore = 0.0;
		double fScore = 0.0;
		Node* cameFrom = NULL;
		int set = 0;
	};

	slabpool<Node> pool;

	Point origin = {0,0,0};
	Point target = {0,0,0};
	Node* candidate = NULL;
	std::map<Point,Node*> nodes;
	std::map<Point,Node*> opens;

	bool done = false;
	bool success = false;
	bool cancel = false;
	std::vector<Point> result;

	Path();
	virtual ~Path();
	void submit();
	Node* getNode(Point point);
	void update();

	bool inOpenSet(Node*);
	void toOpenSet(Node*);
	bool inClosedSet(Node*);
	void toClosedSet(Node*);

	virtual std::vector<Point> getNeighbours(Point) = 0;
	virtual double calcCost(Point,Point) = 0;
	virtual double calcHeuristic(Point) = 0;
	virtual bool rayCast(Point,Point) = 0;
};
