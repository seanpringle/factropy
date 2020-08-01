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

	Mesh meshHD;
	Mesh meshLD;
	Matrix transform;

	Thing();
	Thing(std::string);
	Thing(std::string, std::string);
	Mesh loadSTL(std::string);

	Thing& smooth();
	void drawBatch(Color color, float specular, bool hd, int count, Matrix *trx);
	void drawGhostBatch(Color color, bool hd, int count, Matrix *trx);
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
	float specular;
	Matrix s;
	Matrix r;
	Matrix t;
	Matrix srt;
	bool drawHD;
	bool drawLD;

	Part* ld(bool);
	Part* paint(int colour);
	Part* gloss(float shine);
	Part* rotate(Point axis, float degrees);
	Part* scale(float x, float y, float z);
	Part* translate(float x, float y, float z);
	virtual void update();

	void draw(Matrix trx);
	void drawInstanced(bool hd, int count, Matrix* trx);
	void drawGhostInstanced(bool hd, int count, Matrix* trx);

	Matrix specInstanceState(Spec* spec, uint slot, uint state);
	virtual Matrix specInstance(Spec* spec, uint slot, uint state, Matrix trx);
	virtual Matrix instance(Matrix trx);
};

struct PartSpinner : Part {
	float speed;

	PartSpinner(Thing thing, float speed);
	virtual Matrix specInstance(Spec* spec, uint slot, uint state, Matrix trx);
};

struct PartCycle : Part {
	uint step;
	Matrix shunt;

	PartCycle(Thing thing, uint step);
	virtual void update();
	virtual Matrix specInstance(Spec* spec, uint slot, uint state, Matrix trx);
};

#endif