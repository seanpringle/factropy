#pragma once

// A gridwalk(er) is an iterator for an unbounded 2D plane broken up
// into chunks/tiles of a predefined size. They can iterate from anything
// that has an axis-aligned bounding box.

#include "common.h"
#include "box.h"
#include "point.h"

struct gridwalk {
	Box box = Point::Zero.box();
	int chunk = 1;

	gridwalk(int cchunk, Box bbox) {
		chunk = cchunk;
		box = bbox;
	}

	struct xy {
		int x, y;

		bool operator==(const xy &o) const {
			return x == o.x && y == o.y;
		}

		bool operator<(const xy &o) const {
			return x < o.x || (x == o.x && y < o.y);
		}
	};

	struct iterator {
		int cx0, cy0;
		int cx1, cy1;
		int cx, cy;

		typedef xy value_type;
		typedef std::ptrdiff_t difference_type;
		typedef xy* pointer;
		typedef xy& reference;
		typedef std::input_iterator_tag iterator_category;

		explicit iterator(int chunk, Box box) {
			Point half = Point(box.w, box.h, box.d) * 0.5f;
			Point min = box.centroid() - half;
			Point max = box.centroid() + half;

			cx0 = (int)std::floor(min.x/(float)chunk);
			cy0 = (int)std::floor(min.z/(float)chunk);
			cx1 = (int)std::ceil(max.x/(float)chunk);
			cy1 = (int)std::ceil(max.z/(float)chunk);

			cx = cx0;
			cy = cy0;
		}

		xy operator*() const {
			return {cx,cy};
		}

		bool operator==(const iterator& other) const {
			return cx == other.cx && cy == other.cy;
		}

		bool operator!=(const iterator& other) const {
			return cx != other.cx || cy != other.cy;
		}

		iterator& operator++() {
			cx++;
			if (cx >= cx1) {
				cx = cx0;
				cy++;
			}
			if (cy >= cy1) {
				cx = cx0;
				cy = cy1;
			}
			return *this;
		}
	};

	iterator begin() {
		return iterator(chunk, box);
	}

	iterator end() {
		auto it = iterator(chunk, box);
		it.cx = it.cx0;
		it.cy = it.cy1;
		return it;
	}
};