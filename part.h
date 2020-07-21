#ifndef _H_part
#define _H_part

#include <string>
#include <map>
#include "raylib.h"
#include "raymath.h"

struct Part {
	static void reset();

	static inline Shader shader;
	static inline Material material;
	static inline std::map<std::string,Model> models;

	Mesh mesh;
	Matrix transform;
	Color color;

	Part(std::string obj, Color color);
	virtual ~Part();

	Part* rotate(Vector3 axis, float degrees);
	Part* translate(float x, float y, float z);
	virtual void update();
	virtual Matrix delta(Matrix trx);
	void draw(Matrix trx);
	void drawInstanced(int count, Matrix* trx);
	void drawGhost(Matrix trx);

	static void terrainNormals(Mesh *mesh);
};

struct PartFacer : public Part {
	PartFacer(std::string obj, Color color);
	virtual Matrix delta(Matrix trx);
};

struct PartSpinner : public Part {
	PartSpinner(std::string obj, Color color);
	virtual Matrix delta(Matrix trx);
};

struct PartRoller : public Part {
	Matrix r;
	Matrix t;
	PartRoller(std::string obj, Color color);
	virtual void update();
	virtual Matrix delta(Matrix trx);
};

struct PartWheel : public Part {
	Matrix r;
	Matrix t;
	float s;
	float o;
	PartWheel(std::string obj, Color color);
	PartWheel* speed(float ss);
	PartWheel* steer(float ss);
	virtual void update();
	virtual Matrix delta(Matrix trx);
};

#endif