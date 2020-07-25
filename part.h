#ifndef _H_part
#define _H_part

struct Part;

#include <string>
#include <vector>
#include <map>
#include "raylib.h"
#include "raymath.h"
#include "entity.h"

struct Thing {
	static inline std::vector<Mesh> meshes;

	Mesh mesh;
	Matrix transform;
	Matrix s;
	Matrix r;
	Matrix t;

	Thing();
	Thing(std::string);

	Thing& smooth();
	void drawBatch(Color color, int count, Matrix *trx);
	void drawGhostBatch(Color color, int count, Matrix *trx);
};

struct Part: Thing {
	static inline std::set<Part*> all;

	static void reset();
	static void terrainNormals(Mesh *mesh);

	static inline Shader shader;
	static inline Material material;

	Part(Thing thing);
	virtual ~Part();

	Color color;

	Part* paint(int colour);
	Part* rotate(Point axis, float degrees);
	Part* scale(float x, float y, float z);
	Part* translate(float x, float y, float z);
	virtual void update();

	void drawInstanced(int count, Matrix* trx);
	void drawGhost(Matrix trx);

	virtual Matrix instance(GuiEntity *ge);
};

struct PartSpinner : Part {
	float speed;

	PartSpinner(Thing thing, float speed);
	virtual Matrix instance(GuiEntity *ge);
};


#endif