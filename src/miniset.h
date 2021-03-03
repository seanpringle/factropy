#pragma once

#include <cassert>
#include <new>
#include <type_traits>
#include <typeinfo>
#include "common.h"
#include "minivec.h"

// A compact std::set alternative for small sets of trivial types

template <class V>
class miniset : public minivec<V> {
	static_assert(std::is_trivially_copyable<V>::value, "miniset<is_trivially_copyable>");

public:
	typedef typename minivec<V>::value_type key_type;
	typedef typename minivec<V>::size_type size_type;
	typedef typename minivec<V>::iterator iterator;

	iterator begin() const {
		return minivec<V>::begin();
	}

	iterator end() const {
		return minivec<V>::end();
	}

	iterator find(const key_type& k) const {
		return std::find(begin(), end(), k);
	}

	bool has(const key_type& k) const {
		return find(k) != end();
	}

	bool contains(const key_type& k) const {
		return has(k);
	}

	void insert(const key_type& k) {
		if (!has(k)) {
			minivec<V>::push_back(k);
			std::sort(begin(), end());
		}
	}

	void erase(const key_type& k) {
		minivec<V>::erase(find(k));
		std::sort(begin(), end());
	}

	size_type count(const key_type& k) const {
		return has(k) ? 1: 0;
	}
};