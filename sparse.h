#ifndef _H_sparse
#define _H_sparse

#include <vector>
#include <iostream>
#include <cstdint>

template <class T>
class SparseArray {
public:

	struct Page {
		uint8_t flags[128];
		T cells[1024];

		Page() {
			for (int i = 0; i < 128; i++) {
				flags[i] = 0x00;
			}
		}

		bool full() {
			uint64_t *u2 = (uint64_t*)flags;
			return u2[0] == ~0ULL && u2[1] == ~0ULL;
		}

		bool empty() {
			uint64_t *u2 = (uint64_t*)flags;
			return u2[0] == 0ULL && u2[1] == 0ULL;
		}

		bool isset(int c) {
			return (flags[c/8] & (1<<(c%8))) != 0;
		}

		void set(int c) {
			flags[c/8] |= (1<<(c%8));
		}

		void clr(int c) {
			flags[c/8] &= ~(1<<(c%8));
		}
	};

	std::vector<Page*> pages;

	SparseArray<T>(int size) {
		int width = size/1024 + (size%1024 ? 1:0);
		for (int i = 0; i < width; i++) {
			pages.push_back(NULL);
		}
	}

	int next(bool zero) {
		int width = (int)pages.size();

		for (int p = 0; p < width; p++) {
			Page *page = pages[p];
			if (page != NULL && !page->full()) {
				for (int b = 0; b < 128; b++) {
					if (page->flags[b] != 0xFF) {
						int c = ((!zero && b == 0) ? 1: b*8);
						for (int l = c+8; c < l && c < 1024; c++) {
							if (!page->isset(c)) {
								return p*1024+c;
							}
						}
					}
				}
			}
		}

		for (int p = 0; p < width; p++) {
			Page *page = pages[p];
			if (page == NULL) {
				return (!zero && p == 0 ? 1:0);
			}
		}

		return -1;
	}

	void set(int i, T v) {
		int p = i/1024;
		int c = i%1024;
		Page *page = pages[p];

		if (page == NULL) {
			page = new Page();
			pages[p] = page;
		}

		page->cells[c] = v;
		page->set(c);
	}

	T* ptr(int i) {
		int p = i/1024;
		int c = i%1024;

		Page *page = pages[p];

		if (page == NULL) {
			page = new Page();
			pages[p] = page;
		}

		page->set(c);
		return &page->cells[c];
	}

	T& ref(int i) {
		int p = i/1024;
		int c = i%1024;

		Page *page = pages[p];

		if (page == NULL) {
			page = new Page();
			pages[p] = page;
		}

		page->set(c);
		return page->cells[c];
	}

	bool has(int i) {
		int p = i/1024;
		int c = i%1024;
		Page *page = pages[p];
		return page != NULL && page->isset(c);
	}

	T get(int i) {
		int p = i/1024;
		int c = i%1024;

		Page *page = pages[p];

		if (page == NULL || !page->isset(c)) {
			fatalf("SparseArray cell does not exist %d", i);
		}

		return page->cells[c];
	}

	T drop(int i) {
		int p = i/1024;
		int c = i%1024;

		Page *page = pages[p];

		if (page == NULL || !page->isset(c)) {
			fatalf("SparseArray cell does not exist %d", i);
		}

		T v = page->cells[c];

		page->clr(c);

		if (page->empty()) {
			pages[p] = NULL;
			delete page;
		}

		return v;
	}

	class iter {
		int p;
		int c;
		SparseArray<T> *a;

	public:
		typedef T value_type;
		typedef std::ptrdiff_t difference_type;
		typedef T* pointer;
		typedef T& reference;
		typedef std::input_iterator_tag iterator_category;

		explicit iter(SparseArray<T> *array, int start) {
			p = start/1024;
			c = start%1024;
			a = array;
		}

		int index() {
			return p*1024+c;
		}

		T& operator*() const {
			return a->pages[p]->cells[c];
		}

		bool operator==(const iter& other) const {
			return p == other.p && c == other.c;
		}

		bool operator!=(const iter& other) const {
			return p != other.p || c != other.c;
		}

		iter& operator++() {
			int width = (int)a->pages.size();
			while (true) {
				c++;
				if (c >= 1024) {
					p++;
					while (p < width && a->pages[p] == NULL) {
						p++;
					}
					c = 0;
				}
				if (p >= width) {
					*this = a->end();
					break;
				}
				if (a->pages[p] != NULL && a->pages[p]->isset(c)) {
					break;
				}
			}
			return *this;
		}
	};

	iter begin() {
		int width = (int)pages.size();
		for (int p = 0; p < width; p++) {
			if (pages[p] != NULL) {
				Page *page = pages[p];
				for (int c = 0; c < 1024; c++) {
					if (page->isset(c)) {
						return iter(this, p*1024+c);
					}
				}
			}
		}
		return end();
	}

	iter end() {
		int width = (int)pages.size();
		return iter(this, width*1024+1024);
	}
};

#endif