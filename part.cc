#include "common.h"
#include "sim.h"
#include "part.h"
#include "rlgl.h"
#include <vector>
#include <fstream>
#include <regex>

#include <algorithm>
#include <cctype>
#include <locale>

namespace {

	void ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));
	}

	void rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	void trim(std::string &s) {
		ltrim(s);
		rtrim(s);
	}

//	bool prefixed(const std::string &s, const std::string &p) {
//		return s.find(p) == 0;
//	}
}

Thing::Thing() {
	ZERO(mesh);
	ZERO(transform);
}

Thing::Thing(std::string stl) {
	auto in = std::ifstream(stl);

	struct Triangle {
		size_t count;
		Vector3 vertices[3];
		Vector3 normal;
	};

	std::vector<Triangle> triangles;

	auto facet = std::regex("facet normal ([^ ]+) ([^ ]+) ([^ ]+)");
	auto vertex = std::regex("vertex ([^ ]+) ([^ ]+) ([^ ]+)");
	Triangle triangle; ZERO(triangle);

	for (std::string line; std::getline(in, line);) {
		trim(line);

		std::smatch fm;
		if (std::regex_match(line, fm, facet)) {
			float x = (float)std::stod(fm[1].str());
			float y = (float)std::stod(fm[2].str());
			float z = (float)std::stod(fm[3].str());
			triangle.normal = (Vector3){x,y,z};
			continue;
		}

		std::smatch vm;
		if (std::regex_match(line, vm, vertex)) {
			float x = (float)std::stod(vm[1].str());
			float y = (float)std::stod(vm[2].str());
			float z = (float)std::stod(vm[3].str());
			triangle.vertices[triangle.count++] = (Vector3){x,y,z};

			if (triangle.count == 3) {
				triangles.push_back(triangle);
				triangle.count = 0;
			}

			continue;
		}
	}

	in.close();

	Mesh mesh; ZERO(mesh);
	mesh.vertexCount = (int)triangles.size()*3;
	mesh.triangleCount = (int)triangles.size();
	mesh.vertices = (float *)RL_CALLOC(mesh.vertexCount*3, sizeof(float));
	mesh.texcoords = (float *)RL_CALLOC(mesh.vertexCount*2, sizeof(float));
	mesh.normals = (float *)RL_CALLOC(mesh.vertexCount*3, sizeof(float));

	// raylib models.c DEFAULT_MESH_VERTEX_BUFFERS=7
	mesh.vboId = (unsigned int *)RL_CALLOC(7, sizeof(unsigned int));

	int vCount = 0;
	int nCount = 0;

	for (size_t t = 0; t < triangles.size(); t++) {
		Triangle& triangle = triangles[t];

		for (int v = 0; v < 3; v++) {
			mesh.vertices[vCount++] = triangle.vertices[v].x;
			mesh.vertices[vCount++] = triangle.vertices[v].y;
			mesh.vertices[vCount++] = triangle.vertices[v].z;
		}

		for (size_t v = 0; v < 3; v++) {
			mesh.normals[nCount++] = triangle.normal.x;
			mesh.normals[nCount++] = triangle.normal.y;
			mesh.normals[nCount++] = triangle.normal.z;
		}
	}

  rlLoadMesh(&mesh, false);

	this->mesh = mesh;
	transform = MatrixRotateX(90.0f*DEG2RAD);
}

Thing& Thing::smooth() {
	Part::terrainNormals(&mesh);
	return *this;
}

void Thing::drawBatch(Color color, int count, Matrix *trx) {
	Part::material.shader = Part::shader;
	Part::material.maps[MAP_DIFFUSE].color = color;
	rlDrawMeshInstanced(mesh, Part::material, count, trx);
}

void Thing::drawGhostBatch(Color color, int count, Matrix *trx) {
	Part::material.shader = GetShaderDefault();
	Part::material.maps[MAP_DIFFUSE].color = (Color){
		color.r,
		color.g,
		color.b,
		(unsigned char)((float)color.a*0.5),
	};
	for (int i = 0; i < count; i++) {
		rlDrawMesh(mesh, Part::material, trx[i]);
	}
}

void Part::reset() {
}

struct Vertex {
	Vector3 v;
	static constexpr float epsilon = 0.000001;

	bool operator==(const Vertex &o) const {
		bool match = true;
		match = match && std::abs(v.x - o.v.x) < epsilon;
		match = match && std::abs(v.y - o.v.y) < epsilon;
		match = match && std::abs(v.z - o.v.z) < epsilon;
		return match;
	}

	bool operator<(const Vertex &o) const {
		return Vector3Length(v) < Vector3Length(o.v);
	}
};

void Part::terrainNormals(Mesh *mesh) {

	// Slightly randomize normals so ground appears slightly uneven or rough, even when untextured
	for (int i = 0; i < mesh->vertexCount; i++) {
		Vector3 n = { mesh->normals[(i + 0)*3 + 0], mesh->normals[(i + 0)*3 + 1], mesh->normals[(i + 0)*3 + 2] };
		Vector3 d = { Sim::random()*0.25f, Sim::random()*0.25f, Sim::random()*0.25f };
		n = Vector3Normalize(Vector3Add(n, d));
		mesh->normals[(i + 0)*3 + 0] = n.x;
		mesh->normals[(i + 0)*3 + 1] = n.y;
		mesh->normals[(i + 0)*3 + 2] = n.z;
	}

	// Now sum and average vertex normals so mesh has an overall smooth terrain look
	std::map<Vertex,Vector3> normals;

	for (int i = 0; i < mesh->vertexCount; i++) {
		Vector3 v = { mesh->vertices[(i + 0)*3 + 0], mesh->vertices[(i + 0)*3 + 1], mesh->vertices[(i + 0)*3 + 2] };
		Vector3 n = { mesh->normals[(i + 0)*3 + 0], mesh->normals[(i + 0)*3 + 1], mesh->normals[(i + 0)*3 + 2] };

		Vertex vt = {v};

		if (normals.count(vt) == 1) {
			normals[vt] = Vector3Add(normals[vt], n);
		} else {
			normals[vt] = n;
		}
	}

	for (int i = 0; i < mesh->vertexCount; i++) {
		Vector3 v = { mesh->vertices[(i + 0)*3 + 0], mesh->vertices[(i + 0)*3 + 1], mesh->vertices[(i + 0)*3 + 2] };
		Vertex vt = {v};
		Vector3 n = normals[vt];
		mesh->normals[(i + 0)*3 + 0] = n.x;
		mesh->normals[(i + 0)*3 + 1] = n.y;
		mesh->normals[(i + 0)*3 + 2] = n.z;
	}

	// 2=normals, see rlUpdateMeshAt()
	rlUpdateMesh(*mesh, 2, mesh->vertexCount);
}

Part::Part(Thing thing) {
	mesh = thing.mesh;
	transform = thing.transform;
	color = WHITE;
}

Part::~Part() {
}

Part* Part::paint(int hexcolor) {
	color = GetColor(hexcolor);
	return this;
}

Part* Part::rotate(Point axis, float degrees) {
	transform = MatrixMultiply(transform, MatrixRotate(axis, degrees*DEG2RAD));
	return this;
}

Part* Part::translate(float x, float y, float z) {
	transform = MatrixMultiply(transform, MatrixTranslate(x, y, z));
	return this;
}

void Part::update() {
}

Matrix Part::instance(GuiEntity* ge) {
	return MatrixMultiply(transform, ge->transform());
}

void Part::drawInstanced(int count, Matrix* trx) {
	drawBatch(color, count, trx);
}

void Part::drawGhost(Matrix trx) {
	drawGhostBatch(color, 1, &trx);
}

PartSpinner::PartSpinner(Thing thing) : Part(thing) {
}

Matrix PartSpinner::instance(GuiEntity* ge) {
	Matrix trx = ge->transform();
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

PartRoller::PartRoller(Thing thing) : Part(thing) {
	r = MatrixIdentity();
	t = MatrixIdentity();
}

void PartRoller::update() {
	r = MatrixRotateX(-(float)(Sim::tick%360)*DEG2RAD);
	t = MatrixMultiply(r, transform);
}

Matrix PartRoller::instance(GuiEntity* ge) {
	return MatrixMultiply(t, ge->transform());
}

PartWheel::PartWheel(Thing thing) : Part(thing) {
	s = 0.0f;
	o = 0.0f;
}

PartWheel* PartWheel::speed(float ss) {
	s = ss;
	return this;
}

PartWheel* PartWheel::steer(float ss) {
	o = ss;
	return this;
}

void PartWheel::update() {
	m = MatrixRotateX(-(float)(Sim::tick%360)*DEG2RAD*s);
	m = MatrixMultiply(m, MatrixRotateZ(o*DEG2RAD));
}

Matrix PartWheel::instance(GuiEntity* ge) {
	return (ge->spec->vehicle)
		? MatrixMultiply(MatrixMultiply(m, transform), ge->transform())
		: MatrixMultiply(transform, ge->transform())
	;
}