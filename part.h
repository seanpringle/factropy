#ifndef _H_part
#define _H_part

struct Part;

#include <string>
#include <map>
#include "raylib.h"
#include "raymath.h"
#include "entity.h"

struct Thing {
	Mesh mesh;
	Matrix transform;

	Thing();
	Thing(std::string);

	Thing& smooth();
	void drawBatch(Color color, int count, Matrix *trx);
	void drawGhostBatch(Color color, int count, Matrix *trx);
};

struct Part: Thing {
	static void reset();
	static void terrainNormals(Mesh *mesh);

	static inline Shader shader;
	static inline Material material;

	Part(Thing thing);
	virtual ~Part();

	Color color;

	Part* paint(int colour);
	Part* rotate(Point axis, float degrees);
	Part* translate(float x, float y, float z);
	virtual void update();

	void drawInstanced(int count, Matrix* trx);
	void drawGhost(Matrix trx);

	virtual Matrix instance(GuiEntity *ge);
};

struct PartSpinner : Part {
	PartSpinner(Thing thing);
	virtual Matrix instance(GuiEntity *ge);
};

struct PartRoller : Part {
	Matrix r;
	Matrix t;
	PartRoller(Thing thing);
	virtual void update();
	virtual Matrix instance(GuiEntity *ge);
};

struct PartWheel : Part {
	float s;
	float o;
	Matrix m;
	PartWheel(Thing thing);
	PartWheel* speed(float ss);
	PartWheel* steer(float ss);
	virtual void update();
	virtual Matrix instance(GuiEntity *ge);
};

#endif