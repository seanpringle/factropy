#pragma once

// Vehicle components are ground cars that path-find and drive around the map.
// They can be manualy told to move, or to follow a series of waypoints in patrol
// mode.

struct Vehicle;

#include "entity.h"
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

	struct Waypoint;

	struct DepartCondition {
		virtual bool ready(Waypoint* waypoint, Vehicle *vehicle) = 0;
		virtual ~DepartCondition();
	};

	struct DepartInactivity: DepartCondition {
		int seconds;
		virtual bool ready(Waypoint* waypoint, Vehicle *vehicle);
		virtual ~DepartInactivity();
	};

	struct DepartItem: DepartCondition {
		static const uint Eq = 1;
		static const uint Ne = 2;
		static const uint Lt = 3;
		static const uint Lte = 4;
		static const uint Gt = 5;
		static const uint Gte = 6;

		uint iid;
		uint count;
		uint op;
		virtual bool ready(Waypoint* waypoint, Vehicle *vehicle);
		virtual ~DepartItem();
	};

	struct Waypoint {
		Point position;
		uint stopId;
		std::string stopName;
		std::vector<DepartCondition*> conditions;

		Waypoint(Point pos);
		Waypoint(uint eid);
		~Waypoint();
	};

	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Vehicle> all;
	static Vehicle& create(int id);
	static Vehicle& get(int id);

	int id = 0;
	std::list<Point> path;
	std::list<Waypoint*> waypoints;
	Waypoint* waypoint;
	Route *pathRequest = NULL;
	uint64_t pause = 0;
	bool patrol;
	bool handbrake;

	void destroy();
	void update();
	Waypoint* addWaypoint(Point p);
	Waypoint* addWaypoint(uint eid);
};
