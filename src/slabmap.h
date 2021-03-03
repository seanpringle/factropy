#pragma once

#include <vector>
#include <functional>
#include <cassert>
#include <new>
#include <type_traits>
#include <typeinfo>
#include "common.h"
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

template <class V, auto ID>
class slabmap {
public:

	static inline uint CopyKeys = 1<<0;

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

	#define slabSize (uint)1024

	uint entries = 0;

	std::vector<V*> slabs;
	std::vector<bool*> flags;

	typedef typename std::remove_reference<decltype(std::declval<V>().*ID)>::type K;

	// Index bucket slots that don't store a copy of the key.
	// Smaller index but poorer lookup locality. Better for key types
	// that cannot be copied and destroyed on the fly.

	struct islot {
		uint slab;
		uint cell;

		islot() {
			slab = 0;
			cell = 0;
		}

		islot(uint s, uint c) {
			slab = s;
			cell = c;
		}

		islot(uint s, uint c, const K& k) {
			slab = s;
			cell = c;
		}

		bool match(const std::vector<V*>& slabs, const std::vector<bool*>& flags, const K& k) const {
			assert(slabs.size() > slab && flags.size() > slab);
			assert(flags[slab][cell]);
			return slabs[slab][cell].*ID == k;
		}
	};

	// Index bucket slots that store a copy of the key.
	// Larger index but good lookup locality. Better for simple scaler
	// value key types that can be copied and destroyed on the fly.

	struct kslot {
		uint slab;
		uint cell;
		K key;

		kslot() {
			slab = 0;
			cell = 0;
		}

		kslot(uint s, uint c) {
			slab = s;
			cell = c;
		}

		kslot(uint s, uint c, const K& k) {
			slab = s;
			cell = c;
			key = k;
		}

		bool match(const std::vector<V*>& slabs, const std::vector<bool*>& flags, const K& k) const {
			assert(slabs.size() > slab && flags.size() > slab);
			assert(flags[slab][cell]);
			assert(slabs[slab][cell].*ID == key);
			return key == k;
		}
	};

	// determines how keys are indexed
	typedef typename std::conditional<sizeof(K) <= sizeof(void*),kslot,islot>::type slot;

	bool match(const slot& s, const K& k) const {
		return s.match(slabs, flags, k);
	}

	std::vector<slot> queue;
	std::vector<minivec<slot>> index;

	std::hash<K> hash;

	std::size_t chain(const K& k) const {
		assert(index.size() > 0);
		return hash(k) % index.size();
	}

public:

	slabmap<V,ID>() {
	}

	~slabmap<V,ID>() {
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

		float current = (float)entries/(float)widths[w].power2;

		if (current > load && widths.size()-1 > w) {
			w++;
		}

		if (current < (load*0.5f) && w > 0) {
			w--;
		}

		if (widths[w].prime != index.size()) {
			//notef("reindex %f %u", load, widths[w].prime);
			index.clear();
			index.shrink_to_fit();
			index.resize(widths[w].prime);
			for (uint s = 0; s < slabs.size(); s++) {
				for (uint i = 0; i < slabSize; i++) {
					if (flags[s][i]) {
						auto& k = slabs[s][i].*ID;
						index[chain(k)].push_back(slot(s, i, k));
					}
				}
			}
		}
	}

	void clear() {
		for (uint i = 0; i < slabs.size(); i++) {
			for (uint j = 0; j < slabSize; j++) {
				if (flags[i][j]) {
					std::destroy_at(&slabs[i][j]);
					flags[i][j] = false;
				}
			}
			std::free(slabs[i]);
			std::free(flags[i]);
		}
		slabs.clear();
		slabs.shrink_to_fit();
		flags.clear();
		flags.shrink_to_fit();
		queue.clear();
		queue.shrink_to_fit();
		index.clear();
		index.shrink_to_fit();
		entries = 0;
	}

	bool has(const K& k) const {
		if (!entries) return false;

		for (slot ref: index[chain(k)]) {
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
		if (!entries) return false;

		auto& bucket = index[chain(k)];

		for (auto it = bucket.begin(); it != bucket.end(); it++) {
			auto& ref = *it;
			if (match(ref, k)) {
				std::destroy_at(&slabs[ref.slab][ref.cell]);
				flags[ref.slab][ref.cell] = false;
				queue.push_back(slot(ref.slab, ref.cell));
				bucket.erase(it);
				entries--;

				reindex();
				return true;
			}
		}
		return false;
	}

	V& refer(const K& k) const {
		if (!entries) throw k;

		for (slot ref: index[chain(k)]) {
			if (match(ref, k)) {
				return slabs[ref.slab][ref.cell];
			}
		}
		throw k;
	}

	V& operator[](const K& k) {
		if (!entries) reindex();

		for (slot ref: index[chain(k)]) {
			if (match(ref, k)) {
				return slabs[ref.slab][ref.cell];
			}
		}

		if (!queue.size()) {
			slabs.push_back((V*)std::calloc(slabSize, sizeof(V)));
			flags.push_back((bool*)std::calloc(slabSize, sizeof(bool)));
			for (int i = slabSize-1; i >= 0; i--) {
				queue.push_back(slot(slabs.size()-1, (uint)i));
			}
		}

		slot next = queue.back();

		flags[next.slab][next.cell] = true;
		new (&slabs[next.slab][next.cell]) V;

		slabs[next.slab][next.cell].*ID = k;
		index[chain(k)].push_back(slot(next.slab, next.cell, k));

		entries++;
		queue.pop_back();

		reindex();

		return slabs[next.slab][next.cell];
	}

	class iterator {
		uint si;
		uint ci;
		bool end;
		slabmap<V,ID> *sm;

	public:
		typedef V value_type;
		typedef std::ptrdiff_t difference_type;
		typedef V* pointer;
		typedef V& reference;
		typedef std::input_iterator_tag iterator_category;

		explicit iterator(slabmap<V,ID> *ssm, uint ssi, uint cci, bool eend) {
			sm = ssm;
			si = ssi;
			ci = cci;
			end = eend;
		}

		V& operator*() const {
			return sm->slabs[si][ci];
		}

		bool operator==(const iterator& other) const {
			if (end && !other.end) return false;
			if (other.end && !end) return false;
			if (end && other.end) return true;
			return si == other.si && ci == other.ci;
		}

		bool operator!=(const iterator& other) const {
			if (end && !other.end) return true;
			if (other.end && !end) return true;
			if (end && other.end) return false;
			return si != other.si || ci != other.ci;
		}

		iterator& operator++() {
			while (!end) {
				ci++;
				if (ci == slabSize) {
					ci = 0;
					si++;
				}
				if (sm->slabs.size() <= si) {
					end = true;
					break;
				}
				if (sm->flags[si][ci]) {
					break;
				}
			}
			return *this;
		}

		iterator operator++(int) {
			iterator tmp(*this);
			++*this;
			return tmp;
		};
	};

	iterator begin() {
		uint si = 0;
		uint ci = 0;
		bool end = false;
		for (;;) {
			if (ci == slabSize) {
				ci = 0;
				si++;
			}
			if (slabs.size() <= si) {
				end = true;
				break;
			}
			if (flags[si][ci]) {
				break;
			}
			ci++;
		}
		return iterator(this, si, ci, end);
	}

	iterator end() {
		return iterator(this, 0, 0, true);
	}
};
