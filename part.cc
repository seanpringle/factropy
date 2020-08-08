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
	ZERO(meshHD);
	ZERO(meshLD);
	transform = Mat4::identity;
}

Thing::Thing(std::string hd) {
	meshHD = loadSTL(hd);
	meshLD = meshHD;
	transform = Mat4::rotateX(90.0f*DEG2RAD);
	notef("%s is missing a low-detail mesh!", hd);
}

Thing::Thing(std::string hd, std::string ld) {
	meshHD = loadSTL(hd);
	meshLD = loadSTL(ld);
	transform = Mat4::rotateX(90.0f*DEG2RAD);
}

Thing::Thing(Mesh mesh) {
	meshHD = mesh;
	meshLD = mesh;
	transform = Mat4::identity;
}

Mesh Thing::loadSTL(std::string stl) {
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

	notef("%s %u", stl, triangles.size());

  rlLoadMesh(&mesh, false);
	meshes.push_back(mesh);

	return mesh;
}

Thing& Thing::smooth() {
	Part::terrainNormals(&meshHD);
	Part::terrainNormals(&meshLD);
	return *this;
}

void Thing::drawBatch(Color color, float specular, bool hd, int count, Mat4 *trx) {
	Part::material.shader = Part::shader;
	Part::material.maps[MAP_DIFFUSE].color = color;
	uint specShine  = GetShaderLocation(Part::shader, "specShine");

	SetShaderValue(Part::shader, specShine, &specular, UNIFORM_FLOAT);
	rlDrawMeshInstanced(hd ? meshHD: meshLD, Part::material, count, trx);

	specular = 0.0f;
	SetShaderValue(Part::shader, specShine, &specular, UNIFORM_FLOAT);
}

void Thing::drawParticleBatch(Color color, float specular, bool hd, Mat4 trx, int count, Point *set) {
	Part::material.shader = Part::particleShader;
	Part::material.maps[MAP_DIFFUSE].color = color;
	uint specShine  = GetShaderLocation(Part::particleShader, "specShine");

	SetShaderValue(Part::particleShader, specShine, &specular, UNIFORM_FLOAT);
	rlDrawParticles(hd ? meshHD: meshLD, Part::material, trx, count, (Vector3*)set);

	specular = 0.0f;
	SetShaderValue(Part::particleShader, specShine, &specular, UNIFORM_FLOAT);
}

void Thing::drawGhostBatch(Color color, bool hd, int count, Mat4 *trx) {
	Part::material.shader = GetShaderDefault();
	Part::material.maps[MAP_DIFFUSE].color = (Color){
		color.r,
		color.g,
		color.b,
		(unsigned char)((float)color.a*0.5),
	};
	for (int i = 0; i < count; i++) {
		rlDrawMesh(hd ? meshHD: meshLD, Part::material, trx[i]);
	}
}

void Part::reset() {
	for (auto part: all) {
		delete part;
	}
	for (auto mesh: Thing::meshes) {
		UnloadMesh(mesh);
	}
	all.clear();
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

Part::Part() {
	drawHD = true;
	ZERO(meshHD);
	drawLD = true;
	ZERO(meshLD);
	transform = Mat4::identity;
	s = Mat4::identity;
	r = Mat4::identity;
	t = Mat4::identity;
	srt = Mat4::identity;
	color = WHITE;
	specular = 0.0f;
	all.insert(this);
}

Part::Part(Thing thing) {
	drawHD = true;
	meshHD = thing.meshHD;
	drawLD = true;
	meshLD = thing.meshLD;
	transform = thing.transform;
	s = Mat4::identity;
	r = Mat4::identity;
	t = Mat4::identity;
	srt = Mat4::identity;
	color = WHITE;
	specular = 0.0f;
	all.insert(this);
}

Part::~Part() {
}

Part* Part::ld(bool b) {
	drawLD = b;
	return this;
}

Part* Part::paint(int hexcolor) {
	color = GetColor(hexcolor);
	return this;
}

Part* Part::gloss(float shine) {
	specular = shine;
	return this;
}

Part* Part::rotate(Point axis, float degrees) {
	r = Mat4::rotate(axis, degrees*DEG2RAD);
	srt = (s * r) * t;
	return this;
}

Part* Part::scale(float x, float y, float z) {
	s = Mat4::scale(x, y, z);
	srt = (s * r) * t;
	return this;
}

Part* Part::translate(float x, float y, float z) {
	t = Mat4::translate(x, y, z);
	srt = (s * r) * t;
	return this;
}

void Part::update() {
	srt = (s * r) * t;
}

void Part::draw(Mat4 trx) {
	Mat4 m = transform * srt * trx;
	drawBatch(color, specular, true, 1, &m);
}

Mat4 Part::specInstanceState(Spec* spec, uint slot, uint state) {
	if (spec->states.size() == 0) {
		ensure(state == 0);
	}

	if (spec->states.size() > 0) {
		ensure(spec->states.size() > state);
		return spec->states[state][slot];
	}

	return Mat4::identity;
}

Mat4 Part::specInstance(Spec* spec, uint slot, uint state, Mat4 trx) {
	Mat4 i = specInstanceState(spec, slot, state);
	Mat4 m = (transform * i) * srt;
	return m * trx;
}

Mat4 Part::instance(Mat4 trx) {
	return (transform * srt) * trx;
}

void Part::drawInstanced(bool hd, int count, Mat4* trx) {
	if (!hd && !drawLD) return;
	drawBatch(color, specular, hd, count, trx);
}

void Part::drawGhostInstanced(bool hd, int count, Mat4* trx) {
	if (!hd && !drawLD) return;
	drawGhostBatch(color, hd, count, trx);
}

PartSpinner::PartSpinner(Thing thing, float sspeed) : Part(thing) {
	speed = sspeed;
}

Mat4 PartSpinner::specInstance(Spec* spec, uint slot, uint state, Mat4 trx) {
	Mat4 i = specInstanceState(spec, slot, state);
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
	Mat4 spin = transform * Mat4::rotateY((Sim::tick%360*speed)*DEG2RAD + noise);
	Mat4 m = (spin * i) * srt;
	return m * trx;
}

PartCycle::PartCycle(Thing thing, uint sstep) : Part(thing) {
	step = sstep;
}

void PartCycle::update() {
	Part::update();
	float inc = 0.01 * (float)step;
	uint mod = 100/step;
	shunt = transform * Mat4::translate(0, 0, (float)(Sim::tick%mod)*inc);
}

Mat4 PartCycle::specInstance(Spec* spec, uint slot, uint state, Mat4 trx) {
	Mat4 i = specInstanceState(spec, slot, state);
	Mat4 m = (shunt * i) * srt;
	return m * trx;
}

PartSmoke::PartSmoke(int pm, int ppt, float pr, float er, float sf, float th, float tv, float td, uint ll, uint lu) : Part() {
	particlesMax = pm;
	particlesPerTick = ppt;
	particleRadius = pr;
	emitRadius = er;
	spreadFactor = sf;
	tickDH = th;
	tickDV = tv;
	tickDecay = td;
	lifeLower = ll;
	lifeUpper = lu;

	meshHD = GenMeshSphere(particleRadius,6,6);

	meshLD = meshHD;
	drawLD = false;

	meshes.push_back(meshHD);

	ensure(all.count(this) > 0);

	paint(0x444444ff);

	for (int i = 0; i < particlesMax; i++) {
		particles.push_back({
			offset: Point::Zero,
			spread: Point::Zero,
			tickDV: 0,
			life: 0,
		});
	}
}

void PartSmoke::update() {
	Part::update();

	int create = particlesPerTick;
	uint lifeRange = lifeUpper - lifeLower;

	for (Particle& p: particles) {
		if (p.life < Sim::tick && create > 0) {
			p.life = Sim::tick + lifeLower + std::round(Sim::random() * (float)lifeRange);
			p.offset = (Point::South * emitRadius * std::sqrt(Sim::random())).randomHorizontal();
			p.spread = p.offset * spreadFactor;
			p.tickDV = tickDV;
			create--;
		}
		if (p.life > Sim::tick) {
			p.offset += p.spread;
			p.offset += (Point::South * (Sim::random()*tickDH)).randomHorizontal();
			p.offset += (Point::Up * ((Sim::random()*0.5+0.5)*p.tickDV));
			p.tickDV *= tickDecay;
		}
	}
}

void PartSmoke::drawInstanced(bool hd, int count, Mat4* trx) {
	if (!hd && !drawLD) return;

	std::vector<Point> batch;

	for (int i = 0; i < count; i++) {
		batch.clear();
		for (Particle& p: particles) {
			if (p.life > Sim::tick) {
				batch.push_back(p.offset);
			}
		}
		if (batch.size() > 0) {
			drawParticleBatch(color, specular, hd, trx[i], batch.size(), batch.data());
		}
	}
}
