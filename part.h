#ifndef _H_part
#define _H_part

struct Part;
struct Thing;

#include <string>
#include <vector>
#include <map>
#include "raylib.h"
#include "raymath.h"
#include "spec.h"

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
	Matrix srt;

	Part* paint(int colour);
	Part* rotate(Point axis, float degrees);
	Part* scale(float x, float y, float z);
	Part* translate(float x, float y, float z);
	virtual void update();

	void draw(Matrix trx);
	void drawInstanced(int count, Matrix* trx);
	void drawGhostInstanced(int count, Matrix* trx);

	Matrix instanceState(Spec* spec, uint slot, uint state);
	virtual Matrix instance(Spec* spec, uint slot, uint state, Matrix trx);
};

struct PartSpinner : Part {
	float speed;

	PartSpinner(Thing thing, float speed);
	virtual Matrix instance(Spec* spec, uint slot, uint state, Matrix trx);
};

struct PartCycle : Part {
	uint step;
	Matrix shunt;

	PartCycle(Thing thing, uint step);
	virtual void update();
	virtual Matrix instance(Spec* spec, uint slot, uint state, Matrix trx);
};

#endif