#ifndef _H_missile
#define _H_missile

struct Missile;

//#include

struct Missile {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Missile> all;
	static Missile& create(uint id);
	static Missile& get(uint id);

	uint id;
	uint tid;
	Point aim;
	bool attacking;

	void destroy();
	void update();
	void attack(uint tid);
};

#endif