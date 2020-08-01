#ifndef _H_vehicle
#define _H_vehicle

struct Vehicle;

#include "entity.h"
#include "sparse.h"
#include "path.h"
#include <list>
#include <vector>

struct Vehicle {

	struct Route: Path {
		Vehicle *vehicle;
		Route(Vehicle*);
		virtual std::vector<Point> getNeighbours(Point);
		virtual double calcCost(Point,Point);
		virtual double calcHeuristic(Point);
		virtual bool rayCast(Point,Point);
	};

	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline SparseArray<Vehicle> all = (MaxEntity);
	static Vehicle& create(int id);
	static Vehicle& get(int id);

	int id = 0;
	std::list<Point> path;
	std::list<Point> legs;
	Route *pathRequest = NULL;
	uint64_t pause = 0;

	void destroy();
	void update();
	void addWaypoint(Point p);
};

#endif