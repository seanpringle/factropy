#pragma once

// Parts wrap a Mesh that can be efficiently rendered in an instance batch.
// For performance the rendering thread only renders entities with instanced
// meshes using predefined transformation patterns. Game entities consist
// of one or more parts with various types of pre-defined behaviour:

struct Part;        // static mesh
struct PartSpinner; // simple rotating mesh
struct PartCycle;   // mesh moving through a list of transformation states
struct PartCycle2;  // mesh moving through a list of transformation states
struct PartSmoke;   // particle emitter
struct Thing;

#include "raylib-ex.h"
#include <string>
#include <vector>
#include <map>
#include "mat4.h"
#include "spec.h"

struct Thing {
	static inline std::vector<Mesh> meshes;

	Mesh meshHD;
	Mesh meshLD;
	Mat4 transform;

	Thing();
	Thing(std::string);
	Thing(std::string, std::string);
	Thing(std::string, Mesh ld);
	Thing(Mesh hd);
	Thing(Mesh hd, Mesh ld);
	Mesh loadSTL(std::string);

	Thing& smooth();
	void drawBatch(Color color, float specular, bool hd, int count, Mat4 *trx);
	void drawParticleBatch(Color color, float specular, bool hd, int count, Vector4 *set);
	void drawGhostBatch(Color color, bool hd, int count, Mat4 *trx);
};

struct Part: Thing {
	static inline std::set<Part*> all;

	static void reset();
	static void terrainNormals(Mesh *mesh);

	static inline Shader shader;
	static inline Shader ghostShader;
	static inline Shader particleShader;
	static inline Material material;

	Part();
	Part(Thing thing);
	virtual ~Part();

	Color color;
	float specular;
	Mat4 s;
	Mat4 r;
	Mat4 t;
	Mat4 srt;
	Mat4 tsrt;
	bool drawHD;
	bool drawLD;

	Part* ld(bool);
	Part* paint(int colour);
	Part* gloss(float shine);
	Part* rotate(Point axis, float degrees);
	Part* scale(float x, float y, float z);
	Part* translate(float x, float y, float z);

	virtual void update();

	void draw(Mat4 trx);
	virtual void drawInstanced(bool hd, int count, Mat4* trx);
	void drawGhostInstanced(bool hd, int count, Mat4* trx);

	Mat4 specInstanceState(Spec* spec, uint slot, uint state);
	virtual Mat4 specInstance(Spec* spec, uint slot, uint state, Mat4 trx);
	virtual Mat4 instance(Mat4 trx);

	// info for GuiEntity
	bool pivot;
	Part* pivots(bool state = true);
};

struct PartSpinner : Part {
	float speed;

	PartSpinner(Thing thing, float speed);
	virtual Mat4 specInstance(Spec* spec, uint slot, uint state, Mat4 trx);
};

struct PartCycle : Part {
	uint ticks;
	Mat4 shunt;
	Mat4 ssrt;

	PartCycle(Thing thing, uint ticks);
	virtual void update();
	virtual Mat4 specInstance(Spec* spec, uint slot, uint state, Mat4 trx);
};

struct PartCycle2 : Part {
	Mat4 shunt;
	Mat4 ssrt;
	std::vector<Mat4> transforms;

	PartCycle2(Thing thing, std::vector<Mat4> transforms);
	virtual void update();
	virtual Mat4 specInstance(Spec* spec, uint slot, uint state, Mat4 trx);
};

struct PartSmoke : Part {

	struct Particle {
		Point offset;
		Point spread;
		float tickDV;
		uint64_t life;
	};

	int particlesMax;
	int particlesPerTick;
	float particleRadius;
	float emitRadius;
	float spreadFactor;
	float tickDH;
	float tickDV;
	float tickDecay;
	uint lifeLower;
	uint lifeUpper;
	std::vector<Particle> particles;

	PartSmoke(int particlesMax, int particlesPerTick, float particleRadius, float emitRadius, float spreadFactor, float tickDH, float tickDV, float tickDecay, uint lifeLower, uint lifeUpper);

	virtual void update();
	virtual void drawInstanced(bool hd, int count, Mat4* trx);
};
