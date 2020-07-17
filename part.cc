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

	Model LoadSTL(std::string stl) {
		auto in = std::ifstream(stl);

		struct Triangle {
			size_t count;
			Vector3 vertices[3];
			Vector3 normal;
		};

		std::vector<Triangle> triangles;

		auto facet = std::regex("facet normal ([^ ]+) ([^ ]+) ([^ ]+)");
		auto vertex = std::regex("vertex ([^ ]+) ([^ ]+) ([^ ]+)");
		Triangle triangle = {0};

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

		Mesh mesh = {0};
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

		Model model = LoadModelFromMesh(mesh);
		model.transform = MatrixRotateX(90.0f*DEG2RAD);
		return model;
	}
}

void Part::test() {
}

Part::Part(std::string path, Color color) {
	if (models.count(path) == 0) {
		models[path] = (path.find(".stl") != path.npos) ? LoadSTL(path) : LoadModel(path.c_str());
	}
	Model model = models[path];
	mesh = model.meshes[0];
	material = LoadMaterialDefault();
	material.shader = shader;
	material.maps[MAP_DIFFUSE].color = color;
	transform = model.transform;
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

PartFacer::PartFacer(std::string obj, Color color) : Part(obj, color) {
}

Matrix PartFacer::delta(Matrix trx) {
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
	Matrix r = MatrixRotateY(noise);
	return MatrixMultiply(MatrixMultiply(transform, r), trx);
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
