#include "common.h"
#include "sim.h"
#include "part.h"
#include "rlgl.h"

Part::Part(std::string obj, Color color) {
	if (models.count(obj) == 0) {
		Model model = LoadModel(obj.c_str());
		models[obj] = model;
	}
	Model model = models[obj];
	mesh = model.meshes[0];
	material = LoadMaterialDefault();
	material.shader = shader;
	material.maps[MAP_DIFFUSE].color = color;
	transform = MatrixIdentity();
}

Part::~Part() {
	UnloadMaterial(material);
}

Part* Part::rotate(Vector3 axis, float degrees) {
	transform = MatrixMultiply(transform, MatrixRotate(axis, degrees*DEG2RAD));
	return this;
}

Part* Part::translate(float x, float y, float z) {
	transform = MatrixMultiply(transform, MatrixTranslate(x, y, z));
	return this;
}

Matrix Part::delta(Matrix trx) {
	return MatrixMultiply(transform, trx);
}

void Part::draw(Matrix trx) {
	rlDrawMesh(mesh, material, delta(trx));
}

PartSpinner::PartSpinner(std::string obj, Color color) : Part(obj, color) {
}

Matrix PartSpinner::delta(Matrix trx) {
	float noise = 0.0f;
	noise += trx.m0;
	noise += trx.m4;
	noise += trx.m8;
	noise += trx.m12;
	noise += trx.m1;
	noise += trx.m5;
	noise += trx.m9;
	noise += trx.m13;
	noise += trx.m2;
	noise += trx.m6;
	noise += trx.m10;
	noise += trx.m14;
	noise += trx.m3;
	noise += trx.m7;
	noise += trx.m11;
	noise += trx.m15;
	Matrix r = MatrixRotateY((Sim::tick%360*4)*DEG2RAD + noise);
	return MatrixMultiply(MatrixMultiply(transform, r), trx);
}
