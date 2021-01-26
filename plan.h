#ifndef _H_plan
#define _H_plan

#include "entity.h"

struct Plan {
	Point position;
	std::vector<GuiFakeEntity*> entities;
	std::vector<Point> offsets;

	Plan();
	Plan(Point p);
	~Plan();

	void add(GuiFakeEntity* ge);
	void move(Point p);
	void rotate();
	void floor(float level);
	bool fits();
	bool entityFits(Spec *spec, Point pos, Point dir);
};

#endif