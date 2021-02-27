#ifndef _H_explosion
#define _H_explosion

struct Explosion;

//#include

struct Explosion {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Explosion> all;
	static Explosion& create(uint id);
	static Explosion& get(uint id);

	uint id;
	Health damage;
	float radius;
	float range;
	float rate;

	void destroy();
	void update();

	void define(Health damage, float radius, float rate);
};

#endif