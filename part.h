#ifndef _H_part
#define _H_part

#include <string>
#include <map>
#include "raylib.h"
#include "raymath.h"

struct Part {
	static inline Shader shader;
	static inline std::map<std::string,Model> models;

	Mesh mesh;
	Material material;
	Matrix transform;

	Part(std::string obj, Color color);
	~Part();

	Part* rotate(Vector3 axis, float degrees);
	Part* translate(float x, float y, float z);
	virtual Matrix delta(Matrix trx);
	void draw(Matrix trx);

	static void test();
};

struct PartFacer : public Part {
	PartFacer(std::string obj, Color color);
	virtual Matrix delta(Matrix trx);
};

struct PartSpinner : public Part {
	PartSpinner(std::string obj, Color color);
	virtual Matrix delta(Matrix trx);
};

#endif