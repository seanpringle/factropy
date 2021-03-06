#pragma once

#include <cassert>
#include <new>
#include <type_traits>
#include <typeinfo>
#include "common.h"

// A compact std::vector alternative for trivial types

template <class V>
class minivec {
	//static_assert(std::is_trivially_move_assignable<V>::value, "minivec<is_trivially_move_assignable>");
	static_assert(std::is_trivially_copyable<V>::value, "minivec<is_trivially_copyable>");

public:
	typedef uint size_type;
	typedef V value_type;

	#define vsize (((sizeof(V) + alignof(V) - 1) / alignof(V)) * alignof(V))

	char* buffer;

	V* cell(uint i) const {
		return (V*)(buffer + (vsize * i));
	}

	size_type top;
	size_type cap;

	minivec<value_type>() {
		top = 0;
		cap = 0;
		buffer = nullptr;
	}

	~minivec<value_type>() {
		clear();
	}

	void clear() {
		for (size_type i = 0; i < top; i++) {
			std::destroy_at(cell(i));
		}
		std::free(buffer);
		buffer = nullptr;
		top = 0;
		cap = 0;
	}

	void reserve(size_type n) {
		if (n > cap) {
			size_type oldCap = cap;
			cap = std::max(4U, cap);
			while (n > cap) cap *= 2;
			char* buffer2 = (char*)std::aligned_alloc(alignof(max_align_t), cap * vsize);
			if (buffer) {
				std::memmove(buffer2, buffer, oldCap * vsize);
				std::free(buffer);
			}
			buffer = buffer2;
		}
	}

	size_type capacity() {
		return cap;
	}

	void shrink_to_fit() {
		if (!top && buffer) {
			clear();
		}
	}

	void resize(size_type n) {
		reserve(n);
		while (top < n) {
			*cell(top++) = value_type();
		}
	}

	value_type& at(size_type i) const {
		return *cell(i);
	}

	value_type& operator[](size_type i) const {
		return at(i);
	}

	value_type& front() const {
		return at(0);
	}

	value_type& back() const {
		return at(top > 0 ? top-1: 0);
	}

	value_type* data() const {
		return cell(0);
	}

	bool empty() const {
		assert(top >= 0);
		return top == 0;
	}

	size_type size() const {
		assert(top >= 0);
		return (size_type)top;
	}

	size_type max_size() const {
		return std::numeric_limits<size_type>::max();
	}

	void push_back(value_type s = value_type()) {
		reserve(top+1);
		*cell(top++) = s;
	}

	void pop_back() {
		if (top > 0) {
			std::destroy_at(cell(--top));
		}
	}

	class iterator {
	public:
		size_type ii;
		const minivec* mv;

		typedef V value_type;
		typedef size_type difference_type;
		typedef V* pointer;
		typedef V& reference;
		typedef std::input_iterator_tag iterator_category;

		explicit iterator(const minivec* mmv, size_type iii) {
			mv = mmv;
			ii = std::min(mv->top, iii);
		}

		V& operator*() const {
			return *mv->cell(ii);
		}

		V* operator->() const {
			return mv->cell(ii);
		}

		bool operator==(const iterator& other) const {
			return ii == other.ii;
		}

		bool operator!=(const iterator& other) const {
			return ii != other.ii;
		}

		bool operator<(const iterator& other) const {
			return ii < other.ii;
		}

		bool operator>(const iterator& other) const {
			return ii > other.ii;
		}

		iterator& operator+=(difference_type d) {
			ii += d;
			ii = std::min(mv->top, ii);
			return *this;
		}

		iterator operator+(const iterator& other) const {
			return iterator(mv, ii+other.ii);
		}

		iterator operator+(difference_type d) const {
			return iterator(mv, ii+d);
		}

		iterator& operator-=(difference_type d) {
			ii = d > ii ? 0: ii-d;
			return *this;
		}

		iterator operator-(const iterator& other) const {
			return operator-(other.ii);
		}

		iterator operator-(difference_type d) const {
			return iterator(mv, d > ii ? 0: ii-d);
		}

		difference_type operator-(const iterator& other) {
			return ii - other.ii;
		}

		iterator& operator++() {
			ii = ii < mv->top ? ii+1: mv->top;
			return *this;
		}

		iterator operator++(int) {
			iterator tmp(*this);
			++*this;
			return tmp;
		};

		iterator& operator--() {
			--ii;
			return *this;
		}

		iterator operator--(int) {
			iterator tmp(*this);
			--*this;
			return tmp;
		};
	};

	iterator begin() const {
		return iterator(this, 0);
	}

	iterator end() const {
		return iterator(this, top);
	}

	iterator erase(iterator it, iterator ie) {

		size_type low = it.ii;
		size_type high = ie.ii;

		if (low > high) {
			high = it.ii;
			low = ie.ii;
		}

		if (high > low && low < top && high <= top) {

			for (size_type i = low; i < high; i++) {
				std::destroy_at(cell(i));
			}

			size_type moveDown = (top - low - 1) * vsize;
			std::memmove((void*)cell(low), (void*)cell(high), moveDown);

			top -= (high-low);
		}

		return it;
	}

	iterator erase(iterator it) {
		return erase(it, iterator(this, it.ii+1));
	}
};