#include "common.h"
#include "slabmap.h"
#include <unordered_map>
#include <chrono>

struct item {
	uint id;
	uint val;
};

int main(int argc, char const *argv[]) {

	uint passes = 100;
	uint values = 1000000;

	bool sm = false;
	bool um = false;

	notef("%u", sizeof(std::vector<uint>));

	for (int i = 0; i < argc; i++) {
		if (std::string(argv[i]) == "--slabmap") {
			sm = true;
		}
		if (std::string(argv[i]) == "--unordered_map") {
			um = true;
		}
	}

	std::function<void(std::string, std::function<void(void)>)> bench = [&](auto name, auto fn) {
		auto start = std::chrono::high_resolution_clock::now();
		fn();
		auto elapsed = std::chrono::high_resolution_clock::now() - start;
		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		notef("%s %ld", name, microseconds);
	};

	std::unordered_map<uint,item> umap;

	for (uint i = 0; i < values; i++) {
		umap[i] = (item){.id = i, .val = i%17 };
	}

	slabmap<item,&item::id> smap;
	smap.load = 1.0f;

	for (uint i = 0; i < values; i++) {
		smap[i] = (item){.id = i, .val = i%17 };
	}

	uint64_t n = 0;

	auto umLookup = [&]() {
		n = 0;
		for (uint i = 0; i < passes; i++) {
			for (uint i = 0; i < values; i++) {
				n += umap[i].val;
			}
		}
	};

	auto umIterate = [&]() {
		n = 0;
		for (uint i = 0; i < passes; i++) {
			for (auto it = umap.begin(); it != umap.end(); ++it) {
				n += (*it).second.val;
			}
		}
	};

	auto smLookup = [&]() {
		n = 0;
		for (uint i = 0; i < passes; i++) {
			for (uint i = 0; i < values; i++) {
				n += smap[i].val;
			}
		}
	};

	auto smIterate = [&]() {
		n = 0;
		for (uint i = 0; i < passes; i++) {
			for (auto it = smap.begin(); it != smap.end(); ++it) {
				n += (*it).val;
			}
		}
	};

	if (um) {
		bench("unordered_map lookup", umLookup);
		bench("unordered_map iterate", umIterate);
	}

	if (sm) {
		bench("slabmap lookup", smLookup);
		bench("slabmap iterate", smIterate);
	}

	return 0;
}