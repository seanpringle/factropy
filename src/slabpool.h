#pragma once

#include <vector>
#include <functional>
#include <cassert>
#include <new>
#include <type_traits>
#include <typeinfo>
#include "common.h"

template <class V, uint slabSize = 1024>
class slabpool {

	typedef uint size_type;

	size_type entries = 0;

	class slabpage {
		bool flags[slabSize];

		#define vsize (sizeof(V) + alignof(V))

		// prevents C++ automatically calling V destructors
		char buffer[vsize * slabSize];

		V& cell(uint i) const {
			// because buffer is really a const char*
			const char* p = buffer + (vsize * i);
			// remove the const qualifier
			return *const_cast<V*>(reinterpret_cast<const V*>(p));
		}

	public:
		slabpage() {
			for (uint i = 0; i < slabSize; i++) {
				flags[i] = false;
			}
		}

		~slabpage() {
			for (uint i = 0; i < slabSize; i++) {
				if (flags[i]) drop(i);
			}
		}

		void use(uint i) {
			assert(i < slabSize);
			assert(!flags[i]);
			flags[i] = true;
			new (&cell(i)) V;
		}

		bool used(uint i) {
			assert(i < slabSize);
			return flags[i];
		}

		uint has(V* ptr) {
			V* a = &cell(0);
			V* b = &cell(slabSize-1);
			return ptr >= a && ptr < b;
		}

		void drop(uint i) {
			assert(i < slabSize);
			assert(flags[i]);
			flags[i] = false;
			std::destroy_at(&cell(i));
		}

		V& refer(uint i) {
			assert(i < slabSize);
			assert(flags[i]);
			return cell(i);
		}
	};

	std::vector<slabpage*> slabs;

public:

	struct slabslot {
		uint slab;
		uint cell;

		slabslot() {
			slab = 0;
			cell = 0;
		}

		slabslot(uint s, uint c) {
			slab = s;
			cell = c;
		}
	};

private:
	std::vector<slabslot> queue;

public:
	slabpool<V,slabSize>() {
	}

	~slabpool<V,slabSize>() {
		clear();
	}

	void clear() {
		for (auto& slab: slabs) delete(slab);
		slabs.clear();
		slabs.shrink_to_fit();
		queue.clear();
		queue.shrink_to_fit();
		entries = 0;
	}

	bool empty() const {
		return !entries;
	}

	size_type size() const {
		return entries;
	}

	slabslot requestSlot() {
		if (!queue.size()) {
			slabs.push_back(new slabpage());
			for (int i = slabSize-1; i >= 0; i--) {
				queue.push_back(slabslot(slabs.size()-1, (uint)i));
			}
		}

		slabslot next = queue.back();
		queue.pop_back();

		slabs[next.slab]->use(next.cell);
		entries++;

		return next;
	}

	bool releaseSlot(slabslot slot) {
		slabpage* slab = slabs[slot.slab];
		if (slab->used(slot.cell)) {
			slab->drop(slot.cell);
			assert(entries > 0);
			entries--;
			queue.push_back(slot);
			return true;
		}
		return false;
	}

	V& referSlot(slabslot slot) const {
		return slabs[slot.slab]->refer(slot.cell);
	}

	V& request() {
		return referSlot(requestSlot());
	}

	bool release(const V* v) {
		for (uint i = 0; i < slabs.size(); i++) {
			uint cell = slabs[i]->has(v);
			if (cell < slabSize) {
				return releaseSlot(slabslot(i, cell));
			}
		}
		return false;
	}

	bool release(const V& r) {
		return release(&r);
	}

	class iterator {
		uint si;
		uint ci;
		bool end;
		slabpool<V,slabSize> *sm;

	public:
		typedef V value_type;
		typedef std::ptrdiff_t difference_type;
		typedef V* pointer;
		typedef V& reference;
		typedef std::input_iterator_tag iterator_category;

		explicit iterator(slabpool<V,slabSize> *ssm, uint ssi, uint cci, bool eend) {
			sm = ssm;
			si = ssi;
			ci = cci;
			end = eend;
		}

		slabslot slot() const {
			return slabslot(si, ci);
		}

		V& operator*() const {
			return sm->slabs[si]->refer(ci);
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
				if (sm->slabs[si]->used(ci)) {
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
			if (slabs[si]->used(ci)) {
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
