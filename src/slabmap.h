#pragma once

#include <vector>
#include <functional>
#include <cassert>
#include <new>
#include <type_traits>
#include <typeinfo>
#include "common.h"
#include "slabpool.h"
#include "minivec.h"

// A hash-table/slab-allocator that:
// - stores objects that contain their own key as a hashable field
// - allocates object memory in pages (iteration locality)
// - vectors for hash buckets (lookup locality, higher load factors)
// - allows element removal during forward iteration

// Anecdata comparison with to unordered_map:
// - artificial benchmarks on a particular linux/libc/g++ combination
//   for an arbitrarily chosen small value type...
// - lookup speed slightly increased :-|
// - iteration time halved :-)
// - memory footprint also roughly halved :-D

template <class V, auto ID, uint slabSize = 1024>
class slabmap {
private:

	struct power2prime {
		uint power2;
		uint prime;
	};

	static inline std::vector<power2prime> widths = {
		{ .power2 = 8, .prime = 7 },
		{ .power2 = 16, .prime = 13 },
		{ .power2 = 32, .prime = 31 },
		{ .power2 = 64, .prime = 61,} ,
		{ .power2 = 128, .prime = 127 },
		{ .power2 = 256, .prime = 251 },
		{ .power2 = 512, .prime = 509 },
		{ .power2 = 1024, .prime = 1021 },
		{ .power2 = 2048, .prime = 2039 },
		{ .power2 = 4096, .prime = 4093 },
		{ .power2 = 8192, .prime = 8191 },
		{ .power2 = 16384, .prime = 16381 },
		{ .power2 = 32768, .prime = 32749 },
		{ .power2 = 65536, .prime = 65521 },
		{ .power2 = 131072, .prime = 131071 },
		{ .power2 = 262144, .prime = 262139 },
		{ .power2 = 524288, .prime = 524287 },
		{ .power2 = 1048576, .prime = 1048573 },
	};

	typedef typename std::remove_reference<decltype(std::declval<V>().*ID)>::type K;

	slabpool<V> pool;

	typedef typename slabpool<V>::slabslot slabslot;

	// Index bucket slots that don't store a copy of the key.
	// Smaller index but poorer lookup locality. Better for key types
	// that cannot be copied and destroyed on the fly.

	struct islot {
		slabslot ss;

		islot() {
		}

		islot(slabslot sss) {
			ss = sss;
		}

		islot(slabslot sss, const K& k) : islot(sss) {
		}

		bool match(const slabpool<V>& pool, const K& k) const {
			return pool.referSlot(ss).*ID == k;
		}
	};

	// Index bucket slots that store a copy of the key.
	// Larger index but good lookup locality. Better for simple scaler
	// value key types that can be copied and destroyed on the fly.

	struct kslot {
		slabslot ss;
		K key;

		kslot() {
		}

		kslot(slabslot sss) {
			ss = sss;
		}

		kslot(slabslot sss, const K& k) : kslot(sss) {
			key = k;
		}

		bool match(const slabpool<V>& pool, const K& k) const {
			assert(pool.referSlot(ss).*ID == key);
			return key == k;
		}
	};

	// determines how keys are indexed
	typedef typename std::conditional<sizeof(K) <= sizeof(void*),kslot,islot>::type mslot;

	std::vector<minivec<mslot>> index;

	bool match(const mslot& s, const K& k) const {
		return s.match(pool, k);
	}

	std::hash<K> hash;

	std::size_t chain(const K& k) const {
		assert(index.size() > 0);
		return hash(k) % index.size();
	}

public:

	slabmap<V,ID,slabSize>() {
	}

	~slabmap<V,ID,slabSize>() {
		clear();
	}

	// buckets are vectors, so interating a bit on collision is cheap
	float load = 4.0f;

	void reindex() {
		uint w = 0;

		for (uint i = 0; w == 0 && i < widths.size(); i++) {
			if (widths[i].prime == index.size()) {
				w = i;
			}
		}

		assert(w >= 0);

		float current = (float)pool.size()/(float)widths[w].power2;

		if (current > load && widths.size()-1 > w) {
			w++;
		}

		if (current < (load*0.5f) && w > 0) {
			w--;
		}

		if (widths[w].prime != index.size()) {
			index.clear();
			index.shrink_to_fit();
			index.resize(widths[w].prime);
			for (auto it = pool.begin(); it != pool.end(); ++it) {
				auto& v = *it;
				auto& k = v.*ID;
				auto ss = it.slot();
				index[chain(k)].push_back(mslot(ss, k));
			}
		}
	}

	void clear() {
		pool.clear();
		index.clear();
		index.shrink_to_fit();
	}

	bool has(const K& k) const {
		if (pool.empty()) return false;

		for (mslot ref: index[chain(k)]) {
			if (match(ref, k)) {
				return true;
			}
		}
		return false;
	}

	bool contains(const K& k) const {
		return has(k);
	}

	uint count(const K& k) const {
		return has(k) ? 1: 0;
	}

	bool erase(const K& k) {
		if (pool.empty()) return false;

		auto& bucket = index[chain(k)];

		for (auto it = bucket.begin(); it != bucket.end(); it++) {
			auto& ref = *it;
			if (match(ref, k)) {
				assert(pool.releaseSlot(ref.ss));
				bucket.erase(it);
				reindex();
				return true;
			}
		}
		return false;
	}

	V& refer(const K& k) const {
		if (pool.empty()) throw k;

		for (mslot ref: index[chain(k)]) {
			if (match(ref, k)) {
				return pool.referSlot(ref.ss);
			}
		}
		throw k;
	}

	V& operator[](const K& k) {
		if (pool.empty()) reindex();

		for (mslot ref: index[chain(k)]) {
			if (match(ref, k)) {
				return pool.referSlot(ref.ss);
			}
		}

		slabslot ss = pool.requestSlot();
		V& v = pool.referSlot(ss);

		v.*ID = k;
		index[chain(k)].push_back(mslot(ss, k));

		reindex();

		return v;
	}

	typedef typename slabpool<V>::iterator iterator;

	iterator begin() {
		return pool.begin();
	}

	iterator end() {
		return pool.end();
	}
};
