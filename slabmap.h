#pragma once

#include <list>
#include <vector>
#include <functional>
#include <cassert>

template <class K, class V>
class slabmap {
private:

	static inline uint minWidth = 1<<4;
	static inline uint maxWidth = 1<<16;
	#define slabSize 1024

	uint width = minWidth;
	uint entries = 0;
	float load = 4.0f;

	struct smpair {
		K key;
		V val;
		bool used;

		smpair() {
			memset((uint8_t*)&key, 0, sizeof(K));
			memset((uint8_t*)&val, 0, sizeof(V));
			used = false;
		}

		smpair(K k, V v) {
			key = k;
			val = v;
			used = true;
		}
	};

	struct smref {
		uint slab;
		uint cell;

		smref() {
			slab = 0;
			cell = 0;
		}

		smref(uint s, uint c) {
			slab = s;
			cell = c;
		}
	};

	std::vector<smref> queue;
	std::vector<std::array<smpair,slabSize>> slabs;
	std::vector<std::vector<smref>> index;

	std::size_t chain(const K& k) const {
		std::hash<K> hash;
		return hash(k) % width;
	}

	void reindex(uint w) {
		width = w;
		index.clear();
		index.shrink_to_fit();
		index.resize(width);
		for (uint s = 0; s < slabs.size(); s++) {
			for (int i = 0; i < slabSize; i++) {
				if (slabs[s][i].used) {
					auto& k = slabs[s][i].key;
					index[chain(k)].push_back(smref(s, i));
				}
			}
		}
	}

public:

	slabmap<K,V>() {
	}

	~slabmap<K,V>() {
	}

	void clear() {
		slabs.clear();
		slabs.shrink_to_fit();
		queue.clear();
		queue.shrink_to_fit();
		index.clear();
		index.shrink_to_fit();
		width = minWidth;
		entries = 0;
	}

	bool has(const K& k) const {
		if (!entries) return false;

		for (smref ref: index[chain(k)]) {
			if (slabs[ref.slab][ref.cell].key == k) {
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
			smpair* pair = &slabs[ref.slab][ref.cell];
			assert(pair->used);

			if (pair->key == k) {
				pair->used = false;
				queue.push_back(ref);
				bucket.erase(it);
				entries--;

				if (width > minWidth && (float)entries/(float)width < load) {
					reindex(width/2);
				}

				return true;
			}
		}
		return false;
	}

	V& operator[](const K& k) {
		if (!entries) reindex(minWidth);

		for (smref ref: index[chain(k)]) {
			smpair* pair = &slabs[ref.slab][ref.cell];
			assert(pair->used);
			if (pair->key == k) {
				return pair->val;
			}
		}

		if (!queue.size()) {
			slabs.push_back(std::array<smpair,slabSize>());
			for (int i = slabSize-1; i >= 0; i--) {
				queue.push_back(smref(slabs.size()-1, (uint)i));
			}
		}

		smref next = queue.back();
		slabs[next.slab][next.cell] = smpair();
		smpair* pair = &slabs[next.slab][next.cell];
		pair->key = k;
		pair->used = true;
		queue.pop_back();
		index[chain(k)].push_back(next);
		entries++;

		if ((float)entries/(float)width > load && width < maxWidth)  {
			reindex(width*2);
		}

		return slabs[next.slab][next.cell].val;
	}

	std::vector<K> keys() const {
		std::vector<K> out;
		for (auto& slab: slabs) {
			for (auto& cell: slab) {
				if (cell.used) {
					out.push_back(cell.key);
				}
			}
		}
		return out;
	}

	std::vector<V> vals() const {
		std::vector<V> out;
		for (auto& slab: slabs) {
			for (auto& cell: slab) {
				if (cell.used) {
					out.push_back(cell.val);
				}
			}
		}
		return out;
	}

	std::vector<std::pair<K,V>> pairs() const {
		std::vector<K,V> out;
		for (auto& slab: slabs) {
			for (auto& cell: slab) {
				if (cell.used) {
					out.push_back(std::pair(cell.key,cell.val));
				}
			}
		}
		return out;
	}

	class viter {
		uint si;
		uint ci;
		slabmap<K,V> *sm;

	public:
		typedef V value_type;
		typedef std::ptrdiff_t difference_type;
		typedef V* pointer;
		typedef V& reference;
		typedef std::input_iterator_tag iterator_category;

		explicit viter(slabmap<K,V> *ssm, uint ssi, uint cci) {
			sm = ssm;
			si = ssi;
			ci = cci;
		}

		V& operator*() const {
			return sm->slabs[si][ci].val;
		}

		bool operator==(const viter& other) const {
			return si == other.si && ci == other.ci;
		}

		bool operator!=(const viter& other) const {
			return si != other.si || ci != other.ci;
		}

		viter& operator++() {
			for (;;) {
				ci++;
				if (ci == slabSize) {
					ci = 0;
					si++;
				}
				if (sm->slabs.size() == si) {
					break;
				}
				if (sm->slabs[si][ci].used) {
					break;
				}
			}
			return *this;
		}
	};

	viter begin() {
		uint si = 0;
		uint ci = 0;
		for (;;) {
			if (ci == slabSize) {
				ci = 0;
				si++;
			}
			if (slabs.size() == si) {
				break;
			}
			if (slabs[si][ci].used) {
				break;
			}
			ci++;
		}
		return viter(this, si, ci);
	}

	viter end() {
		return viter(this, slabs.size(), 0);
	}
};
