#include "common.h"
#include "sim.h"
#include "part.h"
#include <vector>
#include <array>
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
	// rotation for raylib Y-up
	transform = Mat4::rotateX(90.0f*DEG2RAD);
	notef("%s is missing a low-detail mesh!", hd);
}

Thing::Thing(std::string hd, std::string ld) {
	meshHD = loadSTL(hd);
	meshLD = loadSTL(ld);
	// rotation for raylib Y-up
	transform = Mat4::rotateX(90.0f*DEG2RAD);
}

Thing::Thing(std::string hd, Mesh ld) {
	meshHD = loadSTL(hd);
	meshLD = ld;
	// rotation for raylib Y-up
	transform = Mat4::rotateX(90.0f*DEG2RAD);
}

Thing::Thing(Mesh mesh) {
	meshHD = mesh;
	meshLD = mesh;
	transform = Mat4::identity;
}

Thing::Thing(Mesh hd, Mesh ld) {
	meshHD = hd;
	meshLD = ld;
	transform = Mat4::identity;
}

Mesh Thing::loadSTL(std::string stl) {

	if (STLs.count(stl)) {
		return STLs[stl];
	}

	struct Triangle {
		size_t count;
		Vector3 vertices[3];
		Vector3 normal;
	};

	std::vector<Triangle> triangles;
	Triangle triangle; ZERO(triangle);

	auto peek = std::ifstream(stl, std::ifstream::binary);
	char tmp[5]; ZERO(tmp);
	peek.read(tmp, 5);
	peek.close();

	if (memcmp(tmp, "solid", 5) == 0) {

		// https://en.wikipedia.org/wiki/STL_%28file_format%29#ASCII_STL
		auto in = std::ifstream(stl);

		struct Triangle {
			size_t count;
			Vector3 vertices[3];
			Vector3 normal;
		};

		auto facet = std::regex("facet normal ([^ ]+) ([^ ]+) ([^ ]+)");
		auto vertex = std::regex("vertex ([^ ]+) ([^ ]+) ([^ ]+)");

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

	} else {

		// https://en.wikipedia.org/wiki/STL_%28file_format%29#Binary_STL
		auto in = std::ifstream(stl, std::ifstream::binary);
		in.seekg(80, in.beg);

		uint32_t count = 0;
		in.read((char*)&count, sizeof(uint32_t));

		char buf[50];

		for (uint i = 0; i < count; i++) {
			in.read(buf, 50);

			triangle.count = 3;
			triangle.normal = *(Vector3*)(buf+0);
			triangle.vertices[0] = *(Vector3*)(buf+12);
			triangle.vertices[1] = *(Vector3*)(buf+24);
			triangle.vertices[2] = *(Vector3*)(buf+36);

			triangles.push_back(triangle);
		}

		in.close();
	}

	Mesh mesh; ZERO(mesh);
	mesh.vertexCount = (int)triangles.size()*3;
	mesh.triangleCount = (int)triangles.size();
	mesh.vertices = (float *)RL_CALLOC(mesh.vertexCount*3, sizeof(float));
	mesh.texcoords = (float *)RL_CALLOC(mesh.vertexCount*2, sizeof(float));
	mesh.normals = (float *)RL_CALLOC(mesh.vertexCount*3, sizeof(float));

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

	STLs[stl] = mesh;
	return mesh;
}

Thing& Thing::smooth() {

	auto normals = [&](Mesh* mesh) {

		struct Vertex {
			Vector3 v;
			const float epsilon = 0.000001;

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

		// Slightly randomize normals so surface appears slightly uneven or rough, even when untextured
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
	};

	normals(&meshHD);
	normals(&meshLD);

	return *this;
}

void Thing::drawBatch(Color color, float specular, bool hd, int count, Mat4 *trx) {
	Part::material.shader = Part::shader;
	Part::material.maps[MAP_DIFFUSE].color = color;

	SetShaderValue(Part::shader, Part::shaderSpecShine, &specular, UNIFORM_FLOAT);
	rlDrawMeshInstanced2(hd ? meshHD: meshLD, Part::material, count, trx);

	specular = 0.0f;
	SetShaderValue(Part::shader, Part::shaderSpecShine, &specular, UNIFORM_FLOAT);
}

void Thing::drawParticleBatch(Color color, float specular, bool hd, int count, Vector4 *set) {
	Part::material.shader = Part::particleShader;
	Part::material.maps[MAP_DIFFUSE].color = color;

	SetShaderValue(Part::particleShader, Part::particleShaderSpecShine, &specular, UNIFORM_FLOAT);
	rlDrawParticles(hd ? meshHD: meshLD, Part::material, count, (Vector4*)set);

	specular = 0.0f;
	SetShaderValue(Part::particleShader, Part::particleShaderSpecShine, &specular, UNIFORM_FLOAT);
}

void Thing::drawGhostBatch(Color color, bool hd, int count, Mat4 *trx) {
	Part::material.shader = Part::ghostShader;
	Part::material.maps[MAP_DIFFUSE].color = color;
	rlDrawMeshInstanced2(hd ? meshHD: meshLD, Part::material, count, trx);
}

void Part::reset() {
	for (auto& part: all) {
		delete part;
	}
	all.clear();
	for (auto& mesh: meshes) {
		UnloadMesh(mesh);
	}
	meshes.clear();
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
	srt = (s * r) * t;
	tsrt = transform * srt;
	color = WHITE;
	specular = 0.0f;
	pivot = false;
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
	srt = (s * r) * t;
	tsrt = transform * srt;
	color = WHITE;
	specular = 0.0f;
	pivot = false;
	all.insert(this);
}

Part::~Part() {
}

Part* Part::ld(bool b) {
	drawLD = b;
	return this;
}

Part* Part::paint(int hexcolor) {
	color = GetColorSRGB(hexcolor);
	return this;
}

Part* Part::gloss(float shine) {
	specular = shine;
	return this;
}

Part* Part::rotate(Point axis, float degrees) {
	r = Mat4::rotate(axis, degrees*DEG2RAD);
	srt = (s * r) * t;
	tsrt = transform * srt;
	return this;
}

Part* Part::scale(float x, float y, float z) {
	s = Mat4::scale(x, y, z);
	srt = (s * r) * t;
	tsrt = transform * srt;
	return this;
}

Part* Part::translate(float x, float y, float z) {
	t = Mat4::translate(x, y, z);
	srt = (s * r) * t;
	tsrt = transform * srt;
	return this;
}

Part* Part::pivots(bool state) {
	pivot = state;
	return this;
}

void Part::update() {
	srt = (s * r) * t;
	tsrt = transform * srt;
}

void Part::draw(Mat4 trx) {
	Mat4 m = tsrt * trx;
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
	if (spec->states.size() > 0) {
		Mat4 i = specInstanceState(spec, slot, state);
		return ((transform * i) * srt) * trx;
	}
	return tsrt * trx;
}

Mat4 Part::instance(Mat4 trx) {
	return tsrt * trx;
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

PartCycle::PartCycle(Thing thing, uint tticks) : Part(thing) {
	ticks = tticks;
}

void PartCycle::update() {
	Part::update();
	float speed = 1.0f/(float)ticks;
	shunt = transform * Mat4::translate(0, 0, (float)(Sim::tick%ticks)*speed);
	ssrt = shunt * srt;
}

Mat4 PartCycle::specInstance(Spec* spec, uint slot, uint state, Mat4 trx) {
	if (spec->states.size() > 0) {
		Mat4 i = specInstanceState(spec, slot, state);
		return ((shunt * i) * srt) * trx;
	}
	return ssrt * trx;
}

PartCycle2::PartCycle2(Thing thing, std::vector<Mat4> ttransforms) : Part(thing) {
	transforms = ttransforms;
}

void PartCycle2::update() {
	Part::update();
	uint i = Sim::tick % transforms.size();
	shunt = transform * transforms[i];
	ssrt = shunt * srt;
}

Mat4 PartCycle2::specInstance(Spec* spec, uint slot, uint state, Mat4 trx) {
	if (spec->states.size() > 0) {
		Mat4 i = specInstanceState(spec, slot, state);
		return ((shunt * i) * srt) * trx;
	}
	return ssrt * trx;
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

	//meshHD = GenMeshSphere(particleRadius,6,6);
	meshHD = GenMeshCube(particleRadius*2.0f,particleRadius*2.0f,particleRadius*2.0f);

	meshLD = meshHD;
	drawLD = false;

	meshes.push_back(meshHD);

	ensure(all.count(this) > 0);

	paint(0x444444ff);

	for (int i = 0; i < particlesMax; i++) {
		particles.push_back({
			.offset = Point::Zero,
			.spread = Point::Zero,
			.tickDV = 0,
			.life = 0,
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

	Vector4* batch = new Vector4[count*particlesMax];
	int used = 0;

	for (int i = 0; i < count; i++) {
		Matrix m = trx[i];
		Point pos = Point::Zero.transform(m);
		float s = Vector3Length((Vector3){m.m0, m.m4, m.m8});

		for (Particle& p: particles) {
			if (p.life > Sim::tick) {
				Point v = pos + p.offset;
				batch[used++] = (Vector4) {
					.x = v.x,
					.y = v.y,
					.z = v.z,
					.w = s,
				};
			}
		}
	}

	if (used > 0) {
		drawParticleBatch(color, specular, hd, used, batch);
	}

	delete[] batch;
}

