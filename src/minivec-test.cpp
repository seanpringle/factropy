#include "common.h"
#include "minivec.h"
#include <vector>
#include <chrono>

struct item {
	uint id;
	uint val;
};

int main(int argc, char const *argv[]) {

	uint passes = 1;
	uint values = 1000;

	bool mv = false;
	bool sv = false;

	notef("%u", sizeof(std::vector<uint>));

	for (int i = 0; i < argc; i++) {
		if (std::string(argv[i]) == "--minivec") {
			mv = true;
		}
		if (std::string(argv[i]) == "--vector") {
			sv = true;
		}
	}

	std::function<void(std::string, std::function<void(void)>)> bench = [&](auto name, auto fn) {
		auto start = std::chrono::high_resolution_clock::now();
		fn();
		auto elapsed = std::chrono::high_resolution_clock::now() - start;
		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		notef("%s %ld", name, microseconds);
	};

	std::vector<uint> uvec;

	for (uint i = 0; i < values; i++) {
		uvec.push_back(i);
	}

	minivec<uint> mvec;

	for (uint i = 0; i < values; i++) {
		mvec.push_back(i);
	}

	uint64_t n = 0;

	auto svLookup = [&]() {
		n = 0;
		for (uint i = 0; i < passes; i++) {
			for (uint i = 0; i < values; i++) {
				n += uvec[i];
			}
		}
	};

	auto svIterate = [&]() {
		n = 0;
		for (uint i = 0; i < passes; i++) {
			for (auto it = uvec.begin(); it != uvec.end(); ++it) {
				n += (*it);
			}
		}
	};

	auto mvLookup = [&]() {
		n = 0;
		for (uint i = 0; i < passes; i++) {
			for (uint i = 0; i < values; i++) {
				n += mvec[i];
			}
		}
	};

	auto mvIterate = [&]() {
		n = 0;
		for (uint i = 0; i < passes; i++) {
			for (auto it = mvec.begin(); it != mvec.end(); ++it) {
				n += (*it);
			}
		}
	};

	if (sv) {
		bench("vector lookup", svLookup);
		bench("vector iterate", svIterate);
	}

	if (mv) {
		bench("minivec lookup", mvLookup);
		bench("minivec iterate", mvIterate);
	}

	return 0;
}