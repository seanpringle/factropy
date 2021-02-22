#pragma once

#include <vector>
#include <functional>
#include <cassert>
#include <new>

// A hash-table/slab-allocator that:
// - stores objects that contain their own key as a field
// - allocates object memory in pages (iteration locality)
// - uses vectors for hash chain buckets for (lookup locality)
// - allows element removal during iteration
// - optimizes for read over write

template <class V, auto ID>
class slabmap {
private:

	#define minWidth (uint)(1<<4)
	#define maxWidth (uint)(1<<16)
	#define slabSize (uint)1024

	uint entries = 0;

	struct item : V {
		item() : V() {
		}
		~item() {
		}
	};

	typedef decltype(std::declval<V>().*ID) K;

	struct slot {
		uint slab;
		uint cell;

		slot() {
			slab = 0;
			cell = 0;
		}

		slot(uint s, uint c) {
			slab = s;
			cell = c;
		}
	};

	std::vector<slot> queue;
	std::vector<item*> slabs;
	std::vector<bool*> flags;
	std::vector<std::vector<slot>> index;

	std::size_t chain(const K& k) const {
		assert(index.size() > 0);
		auto kk = k;
		std::hash<decltype(kk)> hash;
		return hash(kk) % index.size();
	}

public:

	float load = 4.0f;
	bool autogrow = true;

	slabmap<V,ID>() {
	}

	~slabmap<V,ID>() {
		clear();
	}

	void reindex() {

		uint width = std::max(minWidth, (uint)index.size());
		float current = (float)entries/(float)width;

		if (current > load && width < maxWidth) {
			width *= 2;
		}

		if (current < (load*0.5) && width > minWidth) {
			width /= 2;
		}

		if (width != index.size()) {
			index.clear();
			index.shrink_to_fit();
			index.resize(width);
			for (uint s = 0; s < slabs.size(); s++) {
				for (uint i = 0; i < slabSize; i++) {
					if (flags[s][i]) {
						auto& k = slabs[s][i].*ID;
						index[chain(k)].push_back(slot(s, i));
					}
				}
			}
		}
	}

	void clear() {
		for (uint i = 0; i < slabs.size(); i++) {
			for (uint j = 0; j < slabSize; j++) {
				if (flags[i][j]) {
					slabs[i][j].~item();
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
			if (slabs[ref.slab][ref.cell].*ID == k) {
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
			assert(flags[ref.slab][ref.cell]);

			if (slabs[ref.slab][ref.cell].*ID == k) {
				slabs[ref.slab][ref.cell].~item();
				flags[ref.slab][ref.cell] = false;
				queue.push_back(ref);
				bucket.erase(it);
				entries--;

				if (entries && autogrow) reindex();
				if (!entries) clear();

				return true;
			}
		}
		return false;
	}

	V& operator[](const K& k) {
		if (!entries) reindex();

		for (slot ref: index[chain(k)]) {
			assert(flags[ref.slab][ref.cell]);
			if (slabs[ref.slab][ref.cell].*ID == k) {
				return slabs[ref.slab][ref.cell];
			}
		}

		if (!queue.size()) {
			slabs.push_back((item*)std::calloc(slabSize, sizeof(item)));
			flags.push_back((bool*)std::calloc(slabSize, sizeof(bool)));
			for (int i = slabSize-1; i >= 0; i--) {
				queue.push_back(slot(slabs.size()-1, (uint)i));
			}
		}

		slot next = queue.back();
		flags[next.slab][next.cell] = true;
		new (&slabs[next.slab][next.cell]) item;
		slabs[next.slab][next.cell].*ID = k;
		queue.pop_back();
		index[chain(k)].push_back(next);
		entries++;

		if (autogrow) reindex();

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
			return (si == other.si && ci == other.ci) || (end && other.end);
		}

		bool operator!=(const iterator& other) const {
			return !operator==(other);
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
			++tmp;
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
