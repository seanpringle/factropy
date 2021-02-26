#pragma once

#include "common.h"
#include "gridwalk.h"
#include <map>

template <auto CHUNK, typename V>
struct gridmap {

	std::map<gridwalk::xy,std::vector<V>> cells;

	gridmap() {};

	void insert(Box box, V id) {
		for (auto cell: gridwalk(CHUNK, box)) {
			cells[cell].push_back(id);
		}
	}

	void remove(Box box, V id) {
		for (auto cell: gridwalk(CHUNK, box)) {
			auto& v = cells[cell];
			v.erase(std::remove(v.begin(), v.end(), id), v.end());
			if (!v.size()) cells.erase(cell);
		}
	}

	void clear() {
		cells.clear();
	}

	std::vector<V> search(Box box) {
		std::vector<V> hits;
		for (auto cell: gridwalk(CHUNK, box)) {
			if (cells.count(cell)) {
				for (auto id: cells[cell]) {
					hits.push_back(id);
				}
			}
		}
		deduplicate(hits);
		return hits;
	}
};